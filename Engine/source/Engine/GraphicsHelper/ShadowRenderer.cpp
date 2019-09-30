#include "stdafx.h"
#include "ShadowRenderer.h"

#include <limits>

#include "SceneRenderer.h"

#include <Engine/Materials/NullMaterial.h>
#include <Engine/Graphics/Shader.h>
#include <Engine/Graphics/TextureData.h>
#include <Engine/Graphics/Frustum.h>


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
	Config::Settings::Graphics const& graphicsSettings = Config::GetInstance()->GetGraphics();

	I_GraphicsApiContext* const api = Viewport::GetCurrentApiContext();

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

	for (int32 i = 0; i < graphicsSettings.NumCascades; i++)
	{
		//calculate orthographic projection matrix based on cascade
		float cascadeStart = (i == 0) ? 0 : pShadowData->m_Cascades[i - 1].distance / graphicsSettings.CSMDrawDistance;
		float cascadeEnd = pShadowData->m_Cascades[i].distance / graphicsSettings.CSMDrawDistance;
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

		float zNear = -graphicsSettings.CSMDrawDistance;//temp, should be calculated differently

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
		api->SetViewport(ivec2(0), res);
		//Set Framebuffer
		api->BindFramebuffer(pShadowData->m_Cascades[i].fbo);
		//Clear Framebuffer
		api->Clear(E_ClearFlag::Color | E_ClearFlag::Depth);

		//Draw scene with light matrix and null material
		SceneRenderer::GetInstance()->DrawShadow();
	}
}

DirectionalShadowData::DirectionalShadowData(ivec2 Resolution)
{
	Config::Settings::Graphics const& graphicsSettings = Config::GetInstance()->GetGraphics();

	I_GraphicsApiContext* const api = Viewport::GetCurrentApiContext();

	//Calculate cascade distances
	m_Cascades.clear();
	float sizeL = 1;
	float distMult = graphicsSettings.CSMDrawDistance / powf(2.f, static_cast<float>(graphicsSettings.NumCascades - 1));
	for (int32 cascade = 0; cascade < graphicsSettings.NumCascades; cascade++)
	{
		auto data = CascadeData();

		api->GenFramebuffers(1, &(data.fbo));

		data.pTexture = new TextureData(Resolution, E_ColorFormat::Depth, E_ColorFormat::Depth, E_DataType::Float);
		data.pTexture->Build();
		api->BindFramebuffer(data.fbo);
		api->LinkTextureToFboDepth(data.pTexture->GetHandle());
		//only depth components
		api->SetDrawBufferCount(0);
		api->SetReadBufferEnabled(false);

		TextureParameters params(false, true);
		params.wrapS = E_TextureWrapMode::ClampToEdge;
		params.wrapT = E_TextureWrapMode::ClampToEdge;
		data.pTexture->SetParameters(params);

		api->BindFramebuffer(0);

		data.distance = sizeL*distMult;
		sizeL *= 2;

		m_Cascades.push_back(data);
	}
}

DirectionalShadowData::~DirectionalShadowData()
{
	I_GraphicsApiContext* const api = Viewport::GetCurrentApiContext();

	for ( size_t i = 0; i < m_Cascades.size(); ++i )
	{
		api->DeleteRenderBuffers(1, &(m_Cascades[i].fbo));
		SafeDelete(m_Cascades[i].pTexture);
	}
}