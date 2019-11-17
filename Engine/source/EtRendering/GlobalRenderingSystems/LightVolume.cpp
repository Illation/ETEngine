#include "stdafx.h"
#include "LightVolume.h"

#include <EtCore/Content/ResourceManager.h>

#include <Engine/Materials/LightMaterial.h>
#include <Engine/Materials/NullMaterial.h>
#include <Engine/Graphics/Shader.h>
#include <Engine/Graphics/TextureData.h>
#include <Engine/Graphics/Frustum.h>
#include <Engine/GlobalRenderingSystems/GlobalRenderingSystems.h>
#include <Engine/SceneRendering/Gbuffer.h>
#include <Engine/SceneRendering/ShadowRenderer.h>
#include <Engine/SceneRendering/ShadedSceneRenderer.h>


PointLightVolume::PointLightVolume()
{

}
PointLightVolume::~PointLightVolume()
{
	if (!IsInitialized)
		return;
	SafeDelete(m_pMaterial);
	SafeDelete(m_pNullMaterial);
}

void PointLightVolume::Initialize()
{
	m_pMaterial = new LightMaterial();
	m_pMaterial->Initialize();
	m_pNullMaterial = new NullMaterial();
	m_pNullMaterial->Initialize();

	IsInitialized = true;
}
void PointLightVolume::Draw(vec3 pos, float radius, vec3 col)
{
	//Make sure everything is set up
	if (!IsInitialized)
	{
		Initialize();
	}

	//Frustum culling
	Sphere objSphere = Sphere(pos, radius);
	if (render::ShadedSceneRenderer::GetCurrent()->GetCamera().GetFrustum().ContainsSphere(objSphere) == VolumeCheck::OUTSIDE)
	{
		return;
	}

	//mat4 World = etm::translate(pos)*etm::scale(vec3(radius));
	mat4 World = etm::scale( vec3( radius ) )*etm::translate( pos );

	m_pMaterial->SetLight(pos, col, radius);
	m_pMaterial->UploadVariables(World);

	RenderingSystems::Instance()->GetPrimitiveRenderer().Draw<primitives::IcoSphere<2> >();
}

DirectLightVolume::DirectLightVolume(){}
DirectLightVolume::~DirectLightVolume() 
{
}
void DirectLightVolume::Initialize()
{
	m_pShader = ResourceManager::Instance()->GetAssetData<ShaderData>("FwdLightDirectionalShader.glsl"_hash);
	m_pShaderShadowed = ResourceManager::Instance()->GetAssetData<ShaderData>("FwdLightDirectionalShadowShader.glsl"_hash);

	m_IsInitialized = true;
}
void DirectLightVolume::Draw(vec3 dir, vec3 col)
{
	if (!m_IsInitialized)
	{
		Initialize();
	}

	// #todo: avoid getting all the uniform info again and again

	I_GraphicsApiContext* const api = Viewport::GetCurrentApiContext();

	api->SetShader(m_pShader.get());

	m_pShader->Upload("texGBufferB"_hash, 1);
	m_pShader->Upload("texGBufferC"_hash, 2);
	auto gbufferTex = render::ShadedSceneRenderer::GetCurrent()->GetGBuffer().GetTextures();
	for (uint32 i = 0; i < (uint32)gbufferTex.size(); i++)
	{
		api->LazyBindTexture(i, gbufferTex[i]->GetTargetType(), gbufferTex[i]->GetHandle());
	}

	//for position reconstruction
	m_pShader->Upload("viewProjInv"_hash, render::ShadedSceneRenderer::GetCurrent()->GetCamera().GetStatViewProjInv());

	m_pShader->Upload("Direction"_hash, dir);
	m_pShader->Upload("Color"_hash, col);

	RenderingSystems::Instance()->GetPrimitiveRenderer().Draw<primitives::Quad>();
}
void DirectLightVolume::DrawShadowed(vec3 dir, vec3 col, DirectionalShadowData const& shadow)
{
	if (!m_IsInitialized)
	{
		Initialize();
	}

	I_GraphicsApiContext* const api = Viewport::GetCurrentApiContext();

	api->SetShader(m_pShaderShadowed.get());

	m_pShaderShadowed->Upload("texGBufferA"_hash, 0);
	m_pShaderShadowed->Upload("texGBufferB"_hash, 1);
	m_pShaderShadowed->Upload("texGBufferC"_hash, 2);
	auto gbufferTex = render::ShadedSceneRenderer::GetCurrent()->GetGBuffer().GetTextures();
	for (uint32 i = 0; i < (uint32)gbufferTex.size(); i++)
	{
		api->LazyBindTexture(i, gbufferTex[i]->GetTargetType(), gbufferTex[i]->GetHandle());
	}

	//for position reconstruction
	Camera const& cam = render::ShadedSceneRenderer::GetCurrent()->GetCamera();

	m_pShaderShadowed->Upload("projectionA"_hash, cam.GetDepthProjA());
	m_pShaderShadowed->Upload("projectionB"_hash, cam.GetDepthProjB());
	m_pShaderShadowed->Upload("viewProjInv"_hash, cam.GetStatViewProjInv());
	m_pShaderShadowed->Upload("camPos"_hash, cam.GetPosition());

	m_pShaderShadowed->Upload("Direction"_hash, dir);
	m_pShaderShadowed->Upload("Color"_hash, col);

	//shadow info
	m_pShaderShadowed->Upload("PcfSamples"_hash, Config::GetInstance()->GetGraphics().NumPCFSamples);
	m_pShaderShadowed->Upload("Bias"_hash, shadow.m_Bias);

	std::string ligStr = "cascades[";
	for (uint32 i = 0; i < (uint32)shadow.m_Cascades.size(); i++)
	{
		//Light Projection
		m_pShaderShadowed->Upload(GetHash(ligStr + std::to_string(i) + "].LightVP"), shadow.m_Cascades[i].lightVP);

		//Shadow map
		m_pShaderShadowed->Upload(GetHash(ligStr + std::to_string(i) + "].ShadowMap"), static_cast<int32>(3 + i));
		api->LazyBindTexture(3 + i, shadow.m_Cascades[i].pTexture->GetTargetType(), shadow.m_Cascades[i].pTexture->GetHandle());

		//cascade distance
		m_pShaderShadowed->Upload(GetHash(ligStr + std::to_string(i) + "].Distance"), shadow.m_Cascades[i].distance);
	}

	RenderingSystems::Instance()->GetPrimitiveRenderer().Draw<primitives::Quad>();
}