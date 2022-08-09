#include "stdafx.h"
#include "AtmospherePrecompute.h"

#include "GlobalRenderingSystems.h"

#include <EtCore/Content/ResourceManager.h>

#include <EtRHI/GraphicsTypes/Shader.h>
#include <EtRHI/Util/PrimitiveRenderer.h>

#include <EtRendering/PlanetTech/Atmosphere.h>


namespace et {
namespace render {


AtmospherePrecompute::~AtmospherePrecompute()
{
	Unload();
}

void AtmospherePrecompute::Init()
{
	m_Settings = AtmosphereSettings();

	m_pComputeTransmittance = core::ResourceManager::Instance()->GetAssetData<rhi::ShaderData>(
		core::HashString("Shaders/AtmoPreComp/ComputeTransmittance.glsl"));
	m_pComputeDirectIrradiance = core::ResourceManager::Instance()->GetAssetData<rhi::ShaderData>(
		core::HashString("Shaders/AtmoPreComp/ComputeDirectIrradiance.glsl"));
	m_pComputeSingleScattering = core::ResourceManager::Instance()->GetAssetData<rhi::ShaderData>(
		core::HashString("Shaders/AtmoPreComp/ComputeSingleScattering.glsl"));
	m_pComputeScatteringDensity = core::ResourceManager::Instance()->GetAssetData<rhi::ShaderData>(
		core::HashString("Shaders/AtmoPreComp/ComputeScatteringDensity.glsl"));
	m_pComputeIndirectIrradiance = core::ResourceManager::Instance()->GetAssetData<rhi::ShaderData>(
		core::HashString("Shaders/AtmoPreComp/ComputeIndirectIrradiance.glsl"));
	m_pComputeMultipleScattering = core::ResourceManager::Instance()->GetAssetData<rhi::ShaderData>(
		core::HashString("Shaders/AtmoPreComp/ComputeMultipleScattering.glsl"));

	//Computation textures
	m_TexDeltaIrradiance = new rhi::TextureData(m_Settings.INTERNAL2D, ivec2(m_Settings.IRRADIANCE_W, m_Settings.IRRADIANCE_H));
	m_TexDeltaIrradiance->AllocateStorage();
	m_TexDeltaIrradiance->SetParameters(m_Settings.m_TexParams);

	m_TexDeltaRayleigh = new rhi::TextureData(m_Settings.INTERNAL3D, m_Settings.m_ScatteringTexDim.xy, m_Settings.m_ScatteringTexDim.z);
	m_TexDeltaRayleigh->AllocateStorage();
	m_TexDeltaRayleigh->SetParameters(m_Settings.m_TexParams);
	m_TexDeltaMultipleScattering = m_TexDeltaRayleigh; //Multiple scattering and rayleigh share same gpu texture space

	m_TexDeltaMie = new rhi::TextureData(m_Settings.INTERNAL3D, m_Settings.m_ScatteringTexDim.xy, m_Settings.m_ScatteringTexDim.z);
	m_TexDeltaMie->AllocateStorage();
	m_TexDeltaMie->SetParameters(m_Settings.m_TexParams);

	m_TexDeltaScattering = new rhi::TextureData(m_Settings.INTERNAL3D, m_Settings.m_ScatteringTexDim.xy, m_Settings.m_ScatteringTexDim.z);
	m_TexDeltaScattering->AllocateStorage();
	m_TexDeltaScattering->SetParameters(m_Settings.m_TexParams);

	rhi::ContextHolder::GetRenderDevice()->GenFramebuffers(1, &m_FBO);

	m_IsInitialized = true;
}

void AtmospherePrecompute::Unload()//unload textures and fbos needed for precomputation without deleting parameters
{
	if (!m_IsInitialized)
		return;

	delete m_TexDeltaIrradiance;
	m_TexDeltaIrradiance = nullptr;
	delete m_TexDeltaRayleigh;
	m_TexDeltaRayleigh = nullptr;
	m_TexDeltaMultipleScattering = nullptr;
	delete m_TexDeltaMie;
	m_TexDeltaMie = nullptr;
	delete m_TexDeltaScattering;
	m_TexDeltaScattering = nullptr;

	rhi::ContextHolder::GetRenderDevice()->DeleteFramebuffers(1, &m_FBO);
	// #todo also unload shaders, extra functionality for content manager

	//assert(glGetError == 0);

	m_IsInitialized = false;
}

void AtmospherePrecompute::Precalculate(Atmosphere* atmo)
{
	if (!m_IsInitialized)
	{
		Init();
	}

	rhi::I_RenderDevice* const device = rhi::ContextHolder::GetRenderDevice();

	//Precomputation variables
	atmo->m_Params.Upload(m_pComputeTransmittance.get(), "uAtmosphere");
	m_Settings.UploadTextureSize(m_pComputeTransmittance.get());
	atmo->m_Params.Upload(m_pComputeDirectIrradiance.get(), "uAtmosphere");
	m_Settings.UploadTextureSize(m_pComputeDirectIrradiance.get());
	atmo->m_Params.Upload(m_pComputeSingleScattering.get(), "uAtmosphere");
	m_Settings.UploadTextureSize(m_pComputeSingleScattering.get());
	atmo->m_Params.Upload(m_pComputeScatteringDensity.get(), "uAtmosphere");
	m_Settings.UploadTextureSize(m_pComputeScatteringDensity.get());
	atmo->m_Params.Upload(m_pComputeIndirectIrradiance.get(), "uAtmosphere");
	m_Settings.UploadTextureSize(m_pComputeIndirectIrradiance.get());
	atmo->m_Params.Upload(m_pComputeMultipleScattering.get(), "uAtmosphere");
	m_Settings.UploadTextureSize(m_pComputeMultipleScattering.get());

	device->BindFramebuffer(m_FBO);

	int32 numScatteringOrders = 4;
	mat3 luminanceFromRadiance = mat3(); //Might not be needed as we dont precompute luminance
	bool blend = false; //Same here

	//Specific texture initialization
	atmo->m_TexTransmittance = new rhi::TextureData(m_Settings.INTERNAL2D, ivec2(m_Settings.TRANSMITTANCE_W, m_Settings.TRANSMITTANCE_H));
	atmo->m_TexTransmittance->AllocateStorage();
	atmo->m_TexTransmittance->SetParameters(m_Settings.m_TexParams);

	atmo->m_TexIrradiance = new rhi::TextureData(m_Settings.INTERNAL2D, ivec2(m_Settings.IRRADIANCE_W, m_Settings.IRRADIANCE_H));
	atmo->m_TexIrradiance->AllocateStorage();
	atmo->m_TexIrradiance->SetParameters(m_Settings.m_TexParams);

	atmo->m_TexInscatter = new rhi::TextureData(m_Settings.INTERNAL3D, m_Settings.m_ScatteringTexDim.xy, m_Settings.m_ScatteringTexDim.z);
	atmo->m_TexInscatter->AllocateStorage();
	atmo->m_TexInscatter->SetParameters(m_Settings.m_TexParams);

	device->SetBlendEquation(rhi::E_BlendEquation::Add);
	device->SetBlendFunction(rhi::E_BlendFactor::One, rhi::E_BlendFactor::One);
	device->SetBlendEnabled(false);

	device->LinkTextureToFbo(0, atmo->m_TexTransmittance->GetLocation(), 0);
	device->SetDrawBufferCount(1);
	device->SetViewport(ivec2(0), ivec2(m_Settings.TRANSMITTANCE_W, m_Settings.TRANSMITTANCE_H));
	device->SetShader(m_pComputeTransmittance.get());
	rhi::PrimitiveRenderer::Instance().Draw<rhi::primitives::Quad>();

	// Compute the direct irradiance, store it in delta_irradiance_texture and,
	// depending on 'blend', either initialize irradiance_texture_ with zeros or
	// leave it unchanged (we don't want the direct irradiance in
	// irradiance_texture_, but only the irradiance from the sky).
	device->LinkTextureToFbo(0, m_TexDeltaIrradiance->GetLocation(), 0);
	device->LinkTextureToFbo(1, atmo->m_TexIrradiance->GetLocation(), 0);
	device->SetDrawBufferCount(2);
	device->SetViewport(ivec2(0), ivec2(m_Settings.IRRADIANCE_W, m_Settings.IRRADIANCE_H));
	device->SetShader(m_pComputeDirectIrradiance.get());
	m_pComputeDirectIrradiance->Upload("uTexTransmittance"_hash, static_cast<rhi::TextureData const*>(atmo->m_TexTransmittance));
	device->SetBlendEnabled({ false, blend });
	rhi::PrimitiveRenderer::Instance().Draw<rhi::primitives::Quad>();
	device->SetBlendEnabled(false);

	// Compute the rayleigh and mie single scattering, store them in
	// delta_rayleigh_scattering_texture and delta_mie_scattering_texture, and
	// either store them or accumulate them in scattering_texture_ and
	// optional_single_mie_scattering_texture_.
	device->LinkTextureToFbo(0, m_TexDeltaRayleigh->GetLocation(), 0);
	device->LinkTextureToFbo(1, m_TexDeltaMie->GetLocation(), 0);
	device->LinkTextureToFbo(2, atmo->m_TexInscatter->GetLocation(), 0);
	device->SetDrawBufferCount(3);
	device->SetViewport(ivec2(0), m_Settings.m_ScatteringTexDim.xy);
	device->SetViewport(ivec2(0), m_Settings.m_ScatteringTexDim.xy);
	device->SetShader(m_pComputeSingleScattering.get());
	m_pComputeSingleScattering->Upload("luminance_from_radiance"_hash, luminanceFromRadiance);
	m_pComputeSingleScattering->Upload("uTexTransmittance"_hash, static_cast<rhi::TextureData const*>(atmo->m_TexTransmittance));
	for (int32 layer = 0; layer < m_Settings.m_ScatteringTexDim.z; ++layer)
	{
		m_pComputeSingleScattering->Upload("layer"_hash, layer);
		device->SetBlendEnabled({ false, false, blend, blend });
		rhi::PrimitiveRenderer::Instance().Draw<rhi::primitives::Quad>();
		device->SetBlendEnabled(false);
	}

	// Compute the 2nd, 3rd and 4th order of scattering, in sequence.
	for (int32 scatteringOrder = 2; scatteringOrder <= numScatteringOrders; ++scatteringOrder)
	{
		// Compute the scattering density, and store it in
		// delta_scattering_density_texture.
		device->LinkTextureToFbo(0, m_TexDeltaScattering->GetLocation(), 0);
		device->LinkTextureToFbo(1, 0, 0);
		device->LinkTextureToFbo(2, 0, 0);
		device->LinkTextureToFbo(3, 0, 0);
		device->SetDrawBufferCount(1);
		device->SetViewport(ivec2(0), m_Settings.m_ScatteringTexDim.xy);
		device->SetShader(m_pComputeScatteringDensity.get());
		m_pComputeScatteringDensity->Upload("uTexTransmittance"_hash, static_cast<rhi::TextureData const*>(atmo->m_TexTransmittance));
		m_pComputeScatteringDensity->Upload("uTexRayleigh"_hash, static_cast<rhi::TextureData const*>(m_TexDeltaRayleigh));
		m_pComputeScatteringDensity->Upload("uTexDeltaMie"_hash, static_cast<rhi::TextureData const*>(m_TexDeltaMie));
		m_pComputeScatteringDensity->Upload("uTexMultipleScattering"_hash, static_cast<rhi::TextureData const*>(m_TexDeltaMultipleScattering));
		m_pComputeScatteringDensity->Upload("uTexDeltaIrradiance"_hash, static_cast<rhi::TextureData const*>(m_TexDeltaIrradiance));
		m_pComputeScatteringDensity->Upload("scattering_order"_hash, scatteringOrder);
		for (int32 layer = 0; layer < m_Settings.m_ScatteringTexDim.z; ++layer)
		{
			m_pComputeScatteringDensity->Upload("layer"_hash, layer);
			rhi::PrimitiveRenderer::Instance().Draw<rhi::primitives::Quad>();
		}

		// Compute the indirect irradiance, store it in delta_irradiance_texture and
		// accumulate it in irradiance_texture_.
		device->LinkTextureToFbo(0, m_TexDeltaIrradiance->GetLocation(), 0);
		device->LinkTextureToFbo(1, atmo->m_TexIrradiance->GetLocation(), 0);
		device->SetDrawBufferCount(2);
		device->SetViewport(ivec2(0), ivec2(m_Settings.IRRADIANCE_W, m_Settings.IRRADIANCE_H));
		device->SetShader(m_pComputeIndirectIrradiance.get());
		m_pComputeIndirectIrradiance->Upload("luminance_from_radiance"_hash, luminanceFromRadiance);
		m_pComputeIndirectIrradiance->Upload("uTexRayleigh"_hash, static_cast<rhi::TextureData const*>(m_TexDeltaRayleigh));
		m_pComputeIndirectIrradiance->Upload("uTexDeltaMie"_hash, static_cast<rhi::TextureData const*>(m_TexDeltaMie));
		//m_pComputeIndirectIrradiance->Upload("uTexDeltaIrradiance"_hash, static_cast<rhi::TextureData const*>(m_TexDeltaMultipleScattering));
		m_pComputeIndirectIrradiance->Upload("scattering_order"_hash, scatteringOrder);
		device->SetBlendEnabled({ false, true });
		rhi::PrimitiveRenderer::Instance().Draw<rhi::primitives::Quad>();
		device->SetBlendEnabled(false);

		// Compute the multiple scattering, store it in
		// delta_multiple_scattering_texture, and accumulate it in
		// scattering_texture_.
		device->LinkTextureToFbo(0, m_TexDeltaMultipleScattering->GetLocation(), 0);
		device->LinkTextureToFbo(1, atmo->m_TexInscatter->GetLocation(), 0);
		device->SetDrawBufferCount(2);
		device->SetViewport(ivec2(0), m_Settings.m_ScatteringTexDim.xy);
		device->SetShader(m_pComputeMultipleScattering.get());
		m_pComputeMultipleScattering->Upload("luminance_from_radiance"_hash, luminanceFromRadiance);
		m_pComputeMultipleScattering->Upload("uTexTransmittance"_hash, static_cast<rhi::TextureData const*>(atmo->m_TexTransmittance));
		m_pComputeMultipleScattering->Upload("uTexDeltaScatteringDensity"_hash, static_cast<rhi::TextureData const*>(m_TexDeltaScattering));
		for (int32 layer = 0; layer < m_Settings.m_ScatteringTexDim.z; ++layer)
		{
			m_pComputeMultipleScattering->Upload("layer"_hash, layer);
			device->SetBlendEnabled({ false, true });
			rhi::PrimitiveRenderer::Instance().Draw<rhi::primitives::Quad>();
			device->SetBlendEnabled(false);
		}
	}
	device->LinkTextureToFbo(1, 0, 0);
	device->LinkTextureToFbo(2, 0, 0);
	device->LinkTextureToFbo(3, 0, 0);

	device->SetBlendEnabled(false);

	Unload();
}

void AtmospherePrecompute::SetUniforms(rhi::ShaderData* shader, rhi::TextureData* transmittance, rhi::TextureData* scattering, rhi::TextureData* irradiance, rhi::TextureData* mie)
{
	rhi::I_RenderDevice* const device = rhi::ContextHolder::GetRenderDevice();

	shader->Upload("uTexTransmittance"_hash, static_cast<rhi::TextureData const*>(transmittance));
	shader->Upload("uTexScattering"_hash, static_cast<rhi::TextureData const*>(scattering));
	shader->Upload("uTexIrradiance"_hash, static_cast<rhi::TextureData const*>(irradiance));
	shader->Upload("uTexMie"_hash, static_cast<rhi::TextureData const*>(mie));
}

void AtmospherePrecompute::ComputeSpectralRadianceToLuminanceFactors(const std::vector<double>& wavelengths,
	const std::vector<double>& solar_irradiance, double lambda_power, dvec3 &color)
{
	color = dvec3(0);
	CIE& cie = RenderingSystems::Instance()->GetCie();
	int32 dlambda = 1;
	dvec3 lambdaVec = dvec3(AtmosphereSettings::kLambdaR, AtmosphereSettings::kLambdaG, AtmosphereSettings::kLambdaB);
	dvec3 solarRGB = cie.Interpolate(wavelengths, solar_irradiance, lambdaVec);
	for (int lambda = AtmosphereSettings::kLambdaMin; lambda < AtmosphereSettings::kLambdaMax; lambda += dlambda)
	{
		double irradiance = cie.Interpolate(wavelengths, solar_irradiance, lambda);

		dvec3 xyz = cie.GetValue(lambda, AtmosphereSettings::kLambdaMin, AtmosphereSettings::kLambdaMax);
		dvec3 rgb = cie.GetRGB(xyz);

		color = color + (rgb*irradiance / solarRGB * math::pow((double)lambda / lambdaVec, lambda_power));
	}
	color = color * (AtmosphereSettings::MAX_LUMINOUS_EFFICACY * dlambda);
}

void AtmospherePrecompute::ConvertSpectrumToLinearSrgb( const std::vector<double>& wavelengths,
	const std::vector<double>& spectrum, dvec3 &rgb) 
{
	dvec3 xyz = dvec3(0);
	CIE& cie = RenderingSystems::Instance()->GetCie();
	const int32 dlambda = 1;
	for (int lambda = m_Settings.kLambdaMin; lambda < m_Settings.kLambdaMax; lambda += dlambda) 
	{
		double value = cie.Interpolate(wavelengths, spectrum, lambda);
		xyz = xyz + cie.GetValue(lambda, AtmosphereSettings::kLambdaMin, AtmosphereSettings::kLambdaMax) * value;
	}
	rgb = cie.GetRGB(xyz);
}


} // namespace render
} // namespace et
