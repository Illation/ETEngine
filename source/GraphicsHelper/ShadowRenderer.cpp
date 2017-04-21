#include "stdafx.hpp"

#include "ShadowRenderer.hpp"
#include <glm/gtx/transform.hpp>
#include <glm\gtx\quaternion.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtx/euler_angles.hpp>
#include "../Game/Materials/NullMaterial.hpp"
#include "../Graphics/ShaderData.hpp"
#include "../Graphics/TextureData.hpp"
#include "../Graphics/Frustum.h"

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
	//Calculate light camera matrix
	//*****************************
	//view
	glm::vec3 worldPos = pTransform->GetWorldPosition();
	glm::vec3 lookAt = worldPos - pTransform->GetForward();
	glm::vec3 upVec = pTransform->GetUp();// glm::vec3(0, 1, 0);//
	glm::mat4 lightView = glm::lookAtLH(worldPos, lookAt, upVec);

	//transform frustum into light space
	FrustumCorners corners = CAMERA->GetFrustum()->GetCorners();
	corners.Transform(lightView);

	for (int i = 0; i < GRAPHICS.NumCascades; i++)
	{
		//calculate orthographic projection matrix based on cascade
		float cascadeStart = (i == 0) ? 0 : pShadowData->m_Cascades[i - 1].distance / GRAPHICS.CSMDrawDistance;
		float cascadeEnd = pShadowData->m_Cascades[i].distance / GRAPHICS.CSMDrawDistance;
		std::vector<glm::vec3> cascade;
		cascade.push_back(corners.na + (corners.fa - corners.na)*cascadeStart);
		cascade.push_back(corners.nb + (corners.fb - corners.nb)*cascadeStart);
		cascade.push_back(corners.nc + (corners.fc - corners.nc)*cascadeStart);
		cascade.push_back(corners.nd + (corners.fd - corners.nd)*cascadeStart);
		cascade.push_back(corners.fa + (corners.fa - corners.na)*cascadeEnd);
		cascade.push_back(corners.fb + (corners.fb - corners.nb)*cascadeEnd);
		cascade.push_back(corners.fc + (corners.fc - corners.nc)*cascadeEnd);
		cascade.push_back(corners.fd + (corners.fd - corners.nd)*cascadeEnd);

		float left = std::numeric_limits<float>::max();
		float right = std::numeric_limits<float>::lowest();
		float bottom = std::numeric_limits<float>::max();
		float top = std::numeric_limits<float>::lowest();
		float zFar = std::numeric_limits<float>::lowest();

		float zNear = -GRAPHICS.CSMDrawDistance;//temp, should be calculated differently

		for (size_t i = 0; i < cascade.size(); i++)
		{
			if (cascade[i].x < left) left = cascade[i].x;
			if (cascade[i].x > right) right = cascade[i].x;
			if (cascade[i].y < bottom) bottom = cascade[i].y;
			if (cascade[i].y > top) top = cascade[i].y;
			if (cascade[i].z > zFar) zFar = cascade[i].z;
		}

		float mult = 0.25f;
		glm::mat4 lightProjection = glm::ortho(left*mult, right*mult, bottom*mult, top*mult, zNear, zFar*mult);

		//view projection
		m_LightVP = lightProjection*lightView;
		pShadowData->m_Cascades[i].lightVP = m_LightVP;

		//Set viewport
		glm::ivec2 res = pShadowData->m_Cascades[i].pTexture->GetResolution();
		glViewport(0, 0, res.x, res.y);
		//Set Framebuffer
		glBindFramebuffer(GL_FRAMEBUFFER, pShadowData->m_Cascades[i].fbo);
		//Clear Framebuffer
		glClear(GL_DEPTH_BUFFER_BIT);

		//Draw scene with light matrix and null material
		SCENE->DrawShadow();
	}
}

DirectionalShadowData::DirectionalShadowData(glm::ivec2 Resolution)
{
	//Calculate cascade distances
	m_Cascades.clear();
	float sizeL = 1;
	float distMult = GRAPHICS.CSMDrawDistance / pow(2, GRAPHICS.NumCascades - 1);
	for (int cascade = 0; cascade < GRAPHICS.NumCascades; cascade++)
	{
		auto data = CascadeData();

		glGenFramebuffers(1, &(data.fbo));

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

		glBindFramebuffer(GL_FRAMEBUFFER, data.fbo);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
		glDrawBuffer(GL_NONE);
		glReadBuffer(GL_NONE);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		data.pTexture = new TextureData(depthMap, Resolution.x, Resolution.y);

		data.distance = sizeL*distMult;
		sizeL *= 2;

		m_Cascades.push_back(data);
	}
}

DirectionalShadowData::~DirectionalShadowData()
{
	for ( size_t i = 0; i < m_Cascades.size(); ++i )
	{
		glDeleteRenderbuffers(1, &(m_Cascades[i].fbo));
		SafeDelete(m_Cascades[i].pTexture);
	}
}