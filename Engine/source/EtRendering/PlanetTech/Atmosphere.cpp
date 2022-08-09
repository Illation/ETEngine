#include "stdafx.h"
#include "Atmosphere.h"

#include "Planet.h"

#include <EtCore/Content/ResourceManager.h>

#include <EtRHI/GraphicsTypes/Shader.h>
#include <EtRHI/GraphicsTypes/TextureData.h>
#include <EtRHI/Util/PrimitiveRenderer.h>

#include <EtRendering/GraphicsTypes/Frustum.h>
#include <EtRendering/SceneStructure/Light.h>
#include <EtRendering/SceneRendering/ShadedSceneRenderer.h>
#include <EtRendering/SceneRendering/Gbuffer.h>
#include <EtRendering/GlobalRenderingSystems/GlobalRenderingSystems.h>


namespace et {
namespace render {


//============
// Atmosphere
//============


//-------------------------
// Atmosphere::d-tor
//
Atmosphere::~Atmosphere()
{
	if (m_Id != 0u)
	{
		delete m_TexTransmittance;
		delete m_TexIrradiance;
		delete m_TexInscatter;

		m_TexTransmittance = nullptr;
		m_TexIrradiance = nullptr;
		m_TexInscatter = nullptr;
	}
}

//-------------------------
// Atmosphere::Initialize
//
// Load parameters, precalculate look up textures...
//
void Atmosphere::Initialize(core::HashString const parameterAssetId)
{
	m_Id = parameterAssetId;

	m_Params = AtmosphereParameters(m_Id, m_SkyColor, m_SunColor);

	RenderingSystems::Instance()->GetAtmospherPrecompute().Precalculate(this);

	//Load and compile Shaders
	m_Shader = core::ResourceManager::Instance()->GetAssetData<rhi::ShaderData>(core::HashString("Shaders/PostAtmosphere.glsl"));
}

//-------------------------
// Atmosphere::Draw
//
void Atmosphere::Draw(vec3 const& position, float const height, float const groundRadius, vec3 const& sunDir, Gbuffer const& gbuffer) const
{
	ET_ASSERT(!m_Id.IsEmpty(), "Atmosphere wasn't initialized before drawing");
	ET_ASSERT(m_TexTransmittance != nullptr, "Atmosphere wasn't precalculated before drawing");
	ET_ASSERT(m_TexIrradiance != nullptr);
	ET_ASSERT(m_TexInscatter != nullptr);

	rhi::I_RenderDevice* const device = rhi::ContextHolder::GetRenderDevice();

	float const radius = groundRadius + height;
	float const icoRadius = radius / 0.996407747f;//scale up the sphere so the face center reaches the top of the atmosphere

	Camera const& cam = render::ShadedSceneRenderer::GetCurrent()->GetCamera();

	math::Sphere objSphere = math::Sphere(position, radius);
	if (cam.GetFrustum().ContainsSphere(objSphere) == VolumeCheck::OUTSIDE)
	{
		return;
	}

	//mat4 transform = math::translate(position)*math::scale(vec3(icoRadius));
	mat4 transform = math::scale(vec3(icoRadius))*math::translate(position);

	device->SetShader(m_Shader.get());

	m_Shader->Upload("model"_hash, transform);

	m_Shader->Upload("Position"_hash, position);
	m_Shader->Upload("Radius"_hash, radius);
	//m_Shader->Upload("SurfaceRadius"_hash, groundRadius);

	m_Params.Upload(m_Shader.get(), "uAtmosphere");
	RenderingSystems::Instance()->GetAtmospherPrecompute().GetSettings().UploadTextureSize(m_Shader.get());

	//m_Shader->Upload("uSkySpectralRadToLum"_hash, math::vecCast<float>(m_SkyColor));
	//m_Shader->Upload("uSunSpectralRadToLum"_hash, math::vecCast<float>(m_SunColor));

	m_Shader->Upload("uTexIrridiance"_hash, static_cast<rhi::TextureData const*>(m_TexIrradiance));
	m_Shader->Upload("uTexInscatter"_hash, static_cast<rhi::TextureData const*>(m_TexInscatter));
	m_Shader->Upload("uTexTransmittance"_hash, static_cast<rhi::TextureData const*>(m_TexTransmittance));

	m_Shader->Upload("SunDir"_hash, sunDir);
	m_Shader->Upload("uSunSize"_hash, vec2(tan(m_Params.sun_angular_radius), cos(m_Params.sun_angular_radius)));
	//if (light)
	//{
	//	m_Shader->Upload("SunIntensity"_hash, brightness);
	//}

	m_Shader->Upload("uTexGBufferA"_hash, static_cast<rhi::TextureData const*>(gbuffer.GetTextures()[0]));
	//m_Shader->Upload("uTexGBufferB"_hash, static_cast<rhi::TextureData const*>(gbuffer.GetTextures()[1]));
	//m_Shader->Upload("uTexGBufferC"_hash, static_cast<rhi::TextureData const*>(gbuffer.GetTextures()[2]));

	rhi::PrimitiveRenderer::Instance().Draw<rhi::primitives::IcoSphere<3> >();
}


} // namespace render
} // namespace et