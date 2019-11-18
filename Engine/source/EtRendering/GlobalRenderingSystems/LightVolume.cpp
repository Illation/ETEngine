#include "stdafx.h"
#include "LightVolume.h"

#include <EtCore/Content/ResourceManager.h>

#include <EtRendering/Materials/LightMaterial.h>
#include <EtRendering/Materials/NullMaterial.h>
#include <EtRendering/GraphicsTypes/Shader.h>
#include <EtRendering/GraphicsTypes/TextureData.h>
#include <EtRendering/GraphicsTypes/Frustum.h>
#include <EtRendering/GraphicsTypes/DirectionalShadowData.h>
#include <EtRendering/GlobalRenderingSystems/GlobalRenderingSystems.h>
#include <EtRendering/SceneRendering/Gbuffer.h>
#include <EtRendering/SceneRendering/ShadedSceneRenderer.h>


//====================
// Point Light Volume 
//====================


PointLightVolume::~PointLightVolume()
{
	if (!IsInitialized)
		return;
	SafeDelete(m_pMaterial);
}

void PointLightVolume::Initialize()
{
	m_pMaterial = new LightMaterial();
	m_pMaterial->Initialize();

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


//=====================
// Direct Light Volume 
//=====================


void DirectLightVolume::Initialize()
{
	m_Shader = ResourceManager::Instance()->GetAssetData<ShaderData>("FwdLightDirectionalShader.glsl"_hash);
	m_ShaderShadowed = ResourceManager::Instance()->GetAssetData<ShaderData>("FwdLightDirectionalShadowShader.glsl"_hash);

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

	api->SetShader(m_Shader.get());

	m_Shader->Upload("texGBufferB"_hash, 1);
	m_Shader->Upload("texGBufferC"_hash, 2);
	auto gbufferTex = render::ShadedSceneRenderer::GetCurrent()->GetGBuffer().GetTextures();
	for (uint32 i = 0; i < (uint32)gbufferTex.size(); i++)
	{
		api->LazyBindTexture(i, gbufferTex[i]->GetTargetType(), gbufferTex[i]->GetHandle());
	}

	//for position reconstruction
	m_Shader->Upload("viewProjInv"_hash, render::ShadedSceneRenderer::GetCurrent()->GetCamera().GetStatViewProjInv());

	m_Shader->Upload("Direction"_hash, dir);
	m_Shader->Upload("Color"_hash, col);

	RenderingSystems::Instance()->GetPrimitiveRenderer().Draw<primitives::Quad>();
}

void DirectLightVolume::DrawShadowed(vec3 dir, vec3 col, render::DirectionalShadowData const& shadow)
{
	if (!m_IsInitialized)
	{
		Initialize();
	}

	I_GraphicsApiContext* const api = Viewport::GetCurrentApiContext();

	api->SetShader(m_ShaderShadowed.get());

	m_ShaderShadowed->Upload("texGBufferA"_hash, 0);
	m_ShaderShadowed->Upload("texGBufferB"_hash, 1);
	m_ShaderShadowed->Upload("texGBufferC"_hash, 2);
	auto gbufferTex = render::ShadedSceneRenderer::GetCurrent()->GetGBuffer().GetTextures();
	for (uint32 i = 0; i < (uint32)gbufferTex.size(); i++)
	{
		api->LazyBindTexture(i, gbufferTex[i]->GetTargetType(), gbufferTex[i]->GetHandle());
	}

	//for position reconstruction
	Camera const& cam = render::ShadedSceneRenderer::GetCurrent()->GetCamera();

	m_ShaderShadowed->Upload("projectionA"_hash, cam.GetDepthProjA());
	m_ShaderShadowed->Upload("projectionB"_hash, cam.GetDepthProjB());
	m_ShaderShadowed->Upload("viewProjInv"_hash, cam.GetStatViewProjInv());
	m_ShaderShadowed->Upload("camPos"_hash, cam.GetPosition());

	m_ShaderShadowed->Upload("Direction"_hash, dir);
	m_ShaderShadowed->Upload("Color"_hash, col);

	//shadow info
	m_ShaderShadowed->Upload("PcfSamples"_hash, RenderingSystems::Instance()->GetGraphicsSettings().NumPCFSamples);
	m_ShaderShadowed->Upload("Bias"_hash, shadow.GetBias());

	std::vector<render::DirectionalShadowData::CascadeData> const& cascades = shadow.GetCascades();
	for (uint32 cascadeIdx = 0u; cascadeIdx < static_cast<uint32>(cascades.size()); ++cascadeIdx)
	{
		std::string const cascadeStruct = FS("cascades[%u].", cascadeIdx);

		//Light Projection
		m_ShaderShadowed->Upload(GetHash(cascadeStruct + "LightVP"), cascades[cascadeIdx].lightVP);

		//Shadow map
		m_ShaderShadowed->Upload(GetHash(cascadeStruct + "ShadowMap"), static_cast<int32>(3 + cascadeIdx));
		api->LazyBindTexture(3 + cascadeIdx, cascades[cascadeIdx].texture->GetTargetType(), cascades[cascadeIdx].texture->GetHandle());

		//cascade distance
		m_ShaderShadowed->Upload(GetHash(cascadeStruct + "Distance"), cascades[cascadeIdx].distance);
	}

	RenderingSystems::Instance()->GetPrimitiveRenderer().Draw<primitives::Quad>();
}