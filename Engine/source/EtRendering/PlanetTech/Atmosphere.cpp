#include "stdafx.h"
#include "Atmosphere.h"

#include "Planet.h"

#include <EtCore/Content/ResourceManager.h>

#include <EtRendering/GraphicsTypes/Shader.h>
#include <EtRendering/GraphicsTypes/TextureData.h>
#include <EtRendering/GraphicsTypes/Frustum.h>
#include <EtRendering/SceneStructure/Light.h>
#include <EtRendering/SceneRendering/SpriteRenderer.h>
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
	m_pShader = core::ResourceManager::Instance()->GetAssetData<ShaderData>(core::HashString("PostAtmosphere.glsl"));
}

//-------------------------
// Atmosphere::Draw
//
void Atmosphere::Draw(vec3 const& position, float const height, float const groundRadius, vec3 const& sunDir) const
{
	ET_ASSERT(!m_Id.IsEmpty(), "Atmosphere wasn't initialized before drawing");
	ET_ASSERT(m_TexTransmittance != nullptr, "Atmosphere wasn't precalculated before drawing");
	ET_ASSERT(m_TexIrradiance != nullptr);
	ET_ASSERT(m_TexInscatter != nullptr);

	I_GraphicsApiContext* const api = Viewport::GetCurrentApiContext();

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

	api->SetShader(m_pShader.get());

	m_pShader->Upload("model"_hash, transform);

	m_pShader->Upload("Position"_hash, position);
	m_pShader->Upload("Radius"_hash, radius);
	//m_pShader->Upload("SurfaceRadius"_hash, groundRadius);

	m_Params.Upload(m_pShader.get(), "uAtmosphere");
	RenderingSystems::Instance()->GetAtmospherPrecompute().GetSettings().UploadTextureSize(m_pShader.get());

	//m_pShader->Upload("uSkySpectralRadToLum"_hash, math::vecCast<float>(m_SkyColor));
	//m_pShader->Upload("uSunSpectralRadToLum"_hash, math::vecCast<float>(m_SunColor));

	m_pShader->Upload("uTexIrridiance"_hash, static_cast<TextureData const*>(m_TexIrradiance));
	m_pShader->Upload("uTexInscatter"_hash, static_cast<TextureData const*>(m_TexInscatter));
	m_pShader->Upload("uTexTransmittance"_hash, static_cast<TextureData const*>(m_TexTransmittance));

	m_pShader->Upload("SunDir"_hash, sunDir);
	m_pShader->Upload("uSunSize"_hash, vec2(tan(m_Params.sun_angular_radius), cos(m_Params.sun_angular_radius)));
	//if (light)
	//{
	//	m_pShader->Upload("SunIntensity"_hash, brightness);
	//}

	RenderingSystems::Instance()->GetPrimitiveRenderer().Draw<primitives::IcoSphere<3> >();
}


} // namespace render
} // namespace et