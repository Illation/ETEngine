#include "stdafx.hpp"

#include "ShadowRenderer.hpp"
#include "../Materials/NullMaterial.hpp"
#include "../Graphics/ShaderData.hpp"
#include "../Graphics/TextureData.hpp"
#include "../Graphics/Frustum.hpp"
#include "RenderPipeline.hpp"
#include "RenderState.hpp"
#include <limits>

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
	vec3 worldPos = pTransform->GetWorldPosition();
	vec3 lookAt = worldPos - pTransform->GetForward();
	vec3 upVec = pTransform->GetUp();// vec3::UP;//
	mat4 lightView = etm::lookAt(worldPos, lookAt, upVec);

	//transform frustum into light space
	FrustumCorners corners = CAMERA->GetFrustum()->GetCorners();
	corners.Transform(lightView);

	for (int32 i = 0; i < GRAPHICS.NumCascades; i++)
	{
		//calculate orthographic projection matrix based on cascade
		float cascadeStart = (i == 0) ? 0 : pShadowData->m_Cascades[i - 1].distance / GRAPHICS.CSMDrawDistance;
		float cascadeEnd = pShadowData->m_Cascades[i].distance / GRAPHICS.CSMDrawDistance;
		std::vector<vec3> cascade;
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

		for (size_t j = 0; j < cascade.size(); j++)
		{
			if (cascade[j].x < left) left = cascade[j].x;
			if (cascade[j].x > right) right = cascade[j].x;
			if (cascade[j].y < bottom) bottom = cascade[j].y;
			if (cascade[j].y > top) top = cascade[j].y;
			if (cascade[j].z > zFar) zFar = cascade[j].z;
		}

		float mult = 0.25f;
		mat4 lightProjection = etm::orthographic(left*mult, right*mult, bottom*mult, top*mult, zNear, zFar*mult);

		//view projection
		m_LightVP = lightView * lightProjection;
		pShadowData->m_Cascades[i].lightVP = m_LightVP;

		//Set viewport
		ivec2 res = pShadowData->m_Cascades[i].pTexture->GetResolution();
		STATE->SetViewport(ivec2(0), res);
		//Set Framebuffer
		STATE->BindFramebuffer(pShadowData->m_Cascades[i].fbo);
		//Clear Framebuffer
		glClear(GL_DEPTH_BUFFER_BIT);

		//Draw scene with light matrix and null material
		RenderPipeline::GetInstance()->DrawShadow();
	}
}

DirectionalShadowData::DirectionalShadowData(ivec2 Resolution)
{
	//Calculate cascade distances
	m_Cascades.clear();
	float sizeL = 1;
	float distMult = GRAPHICS.CSMDrawDistance / powf(2.f, static_cast<float>(GRAPHICS.NumCascades - 1));
	for (int32 cascade = 0; cascade < GRAPHICS.NumCascades; cascade++)
	{
		auto data = CascadeData();

		glGenFramebuffers(1, &(data.fbo));

		GLuint depthMap;
		glGenTextures(1, &depthMap);
		STATE->BindTexture(GL_TEXTURE_2D, depthMap);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, Resolution.x, Resolution.y, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
		STATE->BindFramebuffer(data.fbo);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
		//only depth components
		glDrawBuffer(GL_NONE);
		glReadBuffer(GL_NONE);
		
		//Texture parameters
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		GLfloat borderColor[] = { 1.0, 1.0, 1.0, 1.0 };
		glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);//shadow map comp mode
		STATE->BindFramebuffer(0);

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