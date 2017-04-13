#include "stdafx.hpp"

#include "ShadowRenderer.hpp"
#include <glm/gtx/transform.hpp>
#include <glm\gtx\quaternion.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtx/euler_angles.hpp>
#include "../Game/Materials/NullMaterial.hpp"
#include "../Graphics/ShaderData.hpp"
#include "../Graphics/TextureData.hpp"

ShadowRenderer::ShadowRenderer(){}
ShadowRenderer::~ShadowRenderer()
{
	SafeDelete(m_pMaterial);
}
void ShadowRenderer::Initialize()
{
	m_pMaterial = new NullMaterial();
	m_pMaterial->Initialize();

	IsInitialized = true;
}

void ShadowRenderer::MapDirectional(TransformComponent *pTransform, DirectionalShadowData *pShadowData)
{
	//Set viewport
	glm::ivec2 res = pShadowData->m_pTexture->GetResolution();
	glViewport(0, 0, res.x, res.y);
	//Set Framebuffer
	glBindFramebuffer(GL_FRAMEBUFFER, pShadowData->m_FBO);
	//Clear Framebuffer
	glClear(GL_DEPTH_BUFFER_BIT);
	//Calculate light camera matrix
	//*****************************
	//view
	glm::vec2 halfDim = pShadowData->m_Dimensions * 0.5f;
	glm::mat4 lightProjection = glm::ortho(
		-halfDim.x, halfDim.x, -halfDim.y, halfDim.y, 
		pShadowData->m_NCP, pShadowData->m_FCP);
	//projection
	glm::vec3 worldPos = pTransform->GetWorldPosition();
	glm::vec3 lookAt = worldPos - pTransform->GetForward();
	glm::vec3 upVec = pTransform->GetUp();// glm::vec3(0, 1, 0);//
	glm::mat4 lightView = glm::lookAtLH(worldPos, lookAt, upVec);
	//view projection
	m_LightVP = lightProjection*lightView;
	pShadowData->m_LightVP = m_LightVP;

	//Draw scene with light matrix and null material
	SCENE->DrawShadow();
}

DirectionalShadowData::DirectionalShadowData(glm::ivec2 Resolution, glm::vec2 Dimensions, float ncp, float fcp)
	:m_Dimensions(Dimensions)
	,m_NCP(ncp)
	,m_FCP(fcp)
{
	glGenFramebuffers(1, &m_FBO);

	GLuint depthMap;
	glGenTextures(1, &depthMap);
	glBindTexture(GL_TEXTURE_2D, depthMap);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, Resolution.x, Resolution.y, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	GLfloat borderColor[] = { 1.0, 1.0, 1.0, 1.0 };
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);

	glBindFramebuffer(GL_FRAMEBUFFER, m_FBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	m_pTexture = new TextureData(depthMap, Resolution.x, Resolution.y);
}

DirectionalShadowData::~DirectionalShadowData()
{
	glDeleteRenderbuffers(1, &m_FBO);
	SafeDelete(m_pTexture);
}