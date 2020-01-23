#include "stdafx.h"
#include "LightVolume.h"

#include <EtCore/Content/ResourceManager.h>

#include <EtRendering/MaterialSystem/MaterialData.h>
#include <EtRendering/GraphicsTypes/Shader.h>
#include <EtRendering/GraphicsTypes/TextureData.h>
#include <EtRendering/GraphicsTypes/Frustum.h>
#include <EtRendering/GraphicsTypes/DirectionalShadowData.h>
#include <EtRendering/GlobalRenderingSystems/GlobalRenderingSystems.h>
#include <EtRendering/SceneRendering/Gbuffer.h>
#include <EtRendering/SceneRendering/ShadedSceneRenderer.h>


namespace et {
namespace render {


//====================
// Point Light Volume 
//====================


void PointLightVolume::Draw(vec3 pos, float radius, vec3 col)
{
	//Make sure everything is set up
	if (m_Material == nullptr)
	{
		m_Material = core::ResourceManager::Instance()->GetAssetData<render::Material>("M_Light.json"_hash);
	}

	//Frustum culling
	math::Sphere objSphere = math::Sphere(pos, radius);
	if (render::ShadedSceneRenderer::GetCurrent()->GetCamera().GetFrustum().ContainsSphere(objSphere) == VolumeCheck::OUTSIDE)
	{
		return;
	}

	ShaderData const* const shader = m_Material->GetShader();
	Viewport::GetCurrentApiContext()->SetShader(shader);

	shader->Upload("Position"_hash, pos); 
	shader->Upload("Color"_hash, col);
	shader->Upload("Radius"_hash, radius);
	shader->Upload("model"_hash, math::scale(vec3(radius))*math::translate(pos));

	RenderingSystems::Instance()->GetPrimitiveRenderer().Draw<primitives::IcoSphere<2> >();
}


//=====================
// Direct Light Volume 
//=====================


void DirectLightVolume::Initialize()
{
	m_Shader = core::ResourceManager::Instance()->GetAssetData<ShaderData>("FwdLightDirectionalShader.glsl"_hash);
	m_ShaderShadowed = core::ResourceManager::Instance()->GetAssetData<ShaderData>("FwdLightDirectionalShadowShader.glsl"_hash);

	m_IsInitialized = true;
}

void DirectLightVolume::Draw(vec3 dir, vec3 col)
{
	if (!m_IsInitialized)
	{
		Initialize();
	}

	I_GraphicsApiContext* const api = Viewport::GetCurrentApiContext();

	api->SetShader(m_Shader.get());

	//for position reconstruction
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
		m_ShaderShadowed->Upload(GetHash(cascadeStruct + "ShadowMap"), static_cast<TextureData const*>(cascades[cascadeIdx].texture));

		//cascade distance
		m_ShaderShadowed->Upload(GetHash(cascadeStruct + "Distance"), cascades[cascadeIdx].distance);
	}

	RenderingSystems::Instance()->GetPrimitiveRenderer().Draw<primitives::Quad>();
}


} // namespace render
} // namespace et
