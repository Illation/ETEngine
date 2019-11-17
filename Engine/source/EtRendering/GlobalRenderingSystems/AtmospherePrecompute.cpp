#include "stdafx.h"
#include "AtmospherePrecompute.h"

#include "GlobalRenderingSystems.h"

#include <EtCore/Content/ResourceManager.h>

#include <Engine/Graphics/Shader.h>
#include <Engine/PlanetTech/Atmosphere.h>


namespace render {


AtmospherePrecompute::AtmospherePrecompute()
{
	Init();
}
AtmospherePrecompute::~AtmospherePrecompute()
{
	Unload();
}

void AtmospherePrecompute::Init()
{
	m_Settings = AtmosphereSettings();

	m_pComputeTransmittance = ResourceManager::Instance()->GetAssetData<ShaderData>("ComputeTransmittance.glsl"_hash);
	m_pComputeDirectIrradiance = ResourceManager::Instance()->GetAssetData<ShaderData>("ComputeDirectIrradiance.glsl"_hash);
	m_pComputeSingleScattering = ResourceManager::Instance()->GetAssetData<ShaderData>("ComputeSingleScattering.glsl"_hash);
	m_pComputeScatteringDensity = ResourceManager::Instance()->GetAssetData<ShaderData>("ComputeScatteringDensity.glsl"_hash);
	m_pComputeIndirectIrradiance = ResourceManager::Instance()->GetAssetData<ShaderData>("ComputeIndirectIrradiance.glsl"_hash);
	m_pComputeMultipleScattering = ResourceManager::Instance()->GetAssetData<ShaderData>("ComputeMultipleScattering.glsl"_hash);

	//Computation textures
	m_TexDeltaIrradiance = new TextureData(ivec2(m_Settings.IRRADIANCE_W, m_Settings.IRRADIANCE_H),
		m_Settings.INTERNAL2D, m_Settings.FORMAT, E_DataType::Float);
	m_TexDeltaIrradiance->Build();
	m_TexDeltaIrradiance->SetParameters(m_Settings.m_TexParams);

	m_TexDeltaRayleigh = new TextureData(m_Settings.m_ScatteringTexDim.xy, m_Settings.INTERNAL3D, m_Settings.FORMAT, E_DataType::Float, m_Settings.m_ScatteringTexDim.z);
	m_TexDeltaRayleigh->Build();
	m_TexDeltaRayleigh->SetParameters(m_Settings.m_TexParams);
	m_TexDeltaMultipleScattering = m_TexDeltaRayleigh; //Multiple scattering and rayleigh share same gpu texture space

	m_TexDeltaMie = new TextureData(m_Settings.m_ScatteringTexDim.xy, m_Settings.INTERNAL3D, m_Settings.FORMAT, E_DataType::Float, m_Settings.m_ScatteringTexDim.z);
	m_TexDeltaMie->Build();
	m_TexDeltaMie->SetParameters(m_Settings.m_TexParams);

	m_TexDeltaScattering = new TextureData(m_Settings.m_ScatteringTexDim.xy, m_Settings.INTERNAL3D, m_Settings.FORMAT, E_DataType::Float, m_Settings.m_ScatteringTexDim.z);
	m_TexDeltaScattering->Build();
	m_TexDeltaScattering->SetParameters(m_Settings.m_TexParams);

	Viewport::GetCurrentApiContext()->GenFramebuffers(1, &m_FBO);

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

	Viewport::GetCurrentApiContext()->DeleteFramebuffers(1, &m_FBO);
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

	I_GraphicsApiContext* const api = Viewport::GetCurrentApiContext();

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

	api->BindFramebuffer(m_FBO);

	int32 numScatteringOrders = 4;
	mat3 luminanceFromRadiance = mat3(); //Might not be needed as we dont precompute luminance
	bool blend = false; //Same here

	//Specific texture initialization
	atmo->m_TexTransmittance = new TextureData(ivec2(m_Settings.TRANSMITTANCE_W, m_Settings.TRANSMITTANCE_H),
		m_Settings.INTERNAL2D, m_Settings.FORMAT, E_DataType::Float);
	atmo->m_TexTransmittance->Build();
	atmo->m_TexTransmittance->SetParameters(m_Settings.m_TexParams);
	atmo->m_TexIrradiance = new TextureData(ivec2(m_Settings.IRRADIANCE_W, m_Settings.IRRADIANCE_H),
		m_Settings.INTERNAL2D, m_Settings.FORMAT, E_DataType::Float);
	atmo->m_TexIrradiance->Build();
	atmo->m_TexIrradiance->SetParameters(m_Settings.m_TexParams);

	atmo->m_TexInscatter = new TextureData(m_Settings.m_ScatteringTexDim.xy,
		m_Settings.INTERNAL3D, m_Settings.FORMAT, E_DataType::Float, m_Settings.m_ScatteringTexDim.z);
	atmo->m_TexInscatter->Build();
	atmo->m_TexInscatter->SetParameters(m_Settings.m_TexParams);

	api->SetBlendEquation(E_BlendEquation::Add);
	api->SetBlendFunction(E_BlendFactor::One, E_BlendFactor::One);
	api->SetBlendEnabled(false);

	api->LinkTextureToFbo(0, atmo->m_TexTransmittance->GetHandle(), 0);
	api->SetDrawBufferCount(1);
	api->SetViewport(ivec2(0), ivec2(m_Settings.TRANSMITTANCE_W, m_Settings.TRANSMITTANCE_H));
	api->SetShader(m_pComputeTransmittance.get());
	RenderingSystems::Instance()->GetPrimitiveRenderer().Draw<primitives::Quad>();

	// Compute the direct irradiance, store it in delta_irradiance_texture and,
	// depending on 'blend', either initialize irradiance_texture_ with zeros or
	// leave it unchanged (we don't want the direct irradiance in
	// irradiance_texture_, but only the irradiance from the sky).
	api->LinkTextureToFbo(0, m_TexDeltaIrradiance->GetHandle(), 0);
	api->LinkTextureToFbo(1, atmo->m_TexIrradiance->GetHandle(), 0);
	api->SetDrawBufferCount(2);
	api->SetViewport(ivec2(0), ivec2(m_Settings.IRRADIANCE_W, m_Settings.IRRADIANCE_H));
	api->SetShader(m_pComputeDirectIrradiance.get());
	api->LazyBindTexture(atmo->m_TexTransmittance->GetHandle(), atmo->m_TexTransmittance->GetTargetType(), atmo->m_TexTransmittance->GetHandle());
	m_pComputeDirectIrradiance->Upload("uTexTransmittance"_hash, static_cast<int32>(atmo->m_TexTransmittance->GetHandle()));
	api->SetBlendEnabled({ false, blend });
	RenderingSystems::Instance()->GetPrimitiveRenderer().Draw<primitives::Quad>();
	api->SetBlendEnabled(false);

	// Compute the rayleigh and mie single scattering, store them in
	// delta_rayleigh_scattering_texture and delta_mie_scattering_texture, and
	// either store them or accumulate them in scattering_texture_ and
	// optional_single_mie_scattering_texture_.
	api->LinkTextureToFbo(0, m_TexDeltaRayleigh->GetHandle(), 0);
	api->LinkTextureToFbo(1, m_TexDeltaMie->GetHandle(), 0);
	api->LinkTextureToFbo(2, atmo->m_TexInscatter->GetHandle(), 0);
	api->SetDrawBufferCount(3);
	api->SetViewport(ivec2(0), m_Settings.m_ScatteringTexDim.xy);
	api->SetViewport(ivec2(0), m_Settings.m_ScatteringTexDim.xy);
	api->SetShader(m_pComputeSingleScattering.get());
	m_pComputeSingleScattering->Upload("luminance_from_radiance"_hash, luminanceFromRadiance);
	api->LazyBindTexture(atmo->m_TexTransmittance->GetHandle(), atmo->m_TexTransmittance->GetTargetType(), atmo->m_TexTransmittance->GetHandle());
	m_pComputeSingleScattering->Upload("uTexTransmittance"_hash, static_cast<int32>(atmo->m_TexTransmittance->GetHandle()));
	for (int32 layer = 0; layer < m_Settings.m_ScatteringTexDim.z; ++layer)
	{
		m_pComputeSingleScattering->Upload("layer"_hash, layer);
		api->SetBlendEnabled({ false, false, blend, blend });
		RenderingSystems::Instance()->GetPrimitiveRenderer().Draw<primitives::Quad>();
		api->SetBlendEnabled(false);
	}

	// Compute the 2nd, 3rd and 4th order of scattering, in sequence.
	for (int32 scatteringOrder = 2; scatteringOrder <= numScatteringOrders; ++scatteringOrder)
	{
		// Compute the scattering density, and store it in
		// delta_scattering_density_texture.
		api->LinkTextureToFbo(0, m_TexDeltaScattering->GetHandle(), 0);
		api->LinkTextureToFbo(1, 0, 0);
		api->LinkTextureToFbo(2, 0, 0);
		api->LinkTextureToFbo(3, 0, 0);
		api->SetDrawBufferCount(1);
		api->SetViewport(ivec2(0), m_Settings.m_ScatteringTexDim.xy);
		api->SetShader(m_pComputeScatteringDensity.get());
		api->LazyBindTexture(atmo->m_TexTransmittance->GetHandle(), atmo->m_TexTransmittance->GetTargetType(), atmo->m_TexTransmittance->GetHandle());
		m_pComputeScatteringDensity->Upload("uTexTransmittance"_hash, static_cast<int32>(atmo->m_TexTransmittance->GetHandle()));
		api->LazyBindTexture(m_TexDeltaRayleigh->GetHandle(), m_TexDeltaRayleigh->GetTargetType(), m_TexDeltaRayleigh->GetHandle());
		m_pComputeScatteringDensity->Upload("uTexRayleigh"_hash, static_cast<int32>(m_TexDeltaRayleigh->GetHandle()));
		api->LazyBindTexture(m_TexDeltaMie->GetHandle(), m_TexDeltaMie->GetTargetType(), m_TexDeltaMie->GetHandle());
		m_pComputeScatteringDensity->Upload("uTexDeltaMie"_hash, static_cast<int32>(m_TexDeltaMie->GetHandle()));
		api->LazyBindTexture(m_TexDeltaMultipleScattering->GetHandle(), m_TexDeltaMultipleScattering->GetTargetType(), m_TexDeltaMultipleScattering->GetHandle());
		m_pComputeScatteringDensity->Upload("uTexMultipleScattering"_hash, static_cast<int32>(m_TexDeltaMultipleScattering->GetHandle()));
		api->LazyBindTexture(m_TexDeltaIrradiance->GetHandle(), m_TexDeltaIrradiance->GetTargetType(), m_TexDeltaIrradiance->GetHandle());
		m_pComputeScatteringDensity->Upload("uTexDeltaIrradiance"_hash, static_cast<int32>(m_TexDeltaIrradiance->GetHandle()));
		m_pComputeScatteringDensity->Upload("scattering_order"_hash, scatteringOrder);
		for (int32 layer = 0; layer < m_Settings.m_ScatteringTexDim.z; ++layer)
		{
			m_pComputeScatteringDensity->Upload("layer"_hash, layer);
			RenderingSystems::Instance()->GetPrimitiveRenderer().Draw<primitives::Quad>();
		}

		// Compute the indirect irradiance, store it in delta_irradiance_texture and
		// accumulate it in irradiance_texture_.
		api->LinkTextureToFbo(0, m_TexDeltaIrradiance->GetHandle(), 0);
		api->LinkTextureToFbo(1, atmo->m_TexIrradiance->GetHandle(), 0);
		api->SetDrawBufferCount(2);
		api->SetViewport(ivec2(0), ivec2(m_Settings.IRRADIANCE_W, m_Settings.IRRADIANCE_H));
		api->SetShader(m_pComputeIndirectIrradiance.get());
		m_pComputeIndirectIrradiance->Upload("luminance_from_radiance"_hash, luminanceFromRadiance);
		api->LazyBindTexture(m_TexDeltaRayleigh->GetHandle(), m_TexDeltaRayleigh->GetTargetType(), m_TexDeltaRayleigh->GetHandle());
		m_pComputeIndirectIrradiance->Upload("uTexRayleigh"_hash, static_cast<int32>(m_TexDeltaRayleigh->GetHandle()));
		api->LazyBindTexture(m_TexDeltaMie->GetHandle(), m_TexDeltaMie->GetTargetType(), m_TexDeltaMie->GetHandle());
		m_pComputeIndirectIrradiance->Upload("uTexDeltaMie"_hash, static_cast<int32>(m_TexDeltaMie->GetHandle()));
		api->LazyBindTexture(m_TexDeltaMultipleScattering->GetHandle(), m_TexDeltaMultipleScattering->GetTargetType(), m_TexDeltaMultipleScattering->GetHandle());
		//m_pComputeIndirectIrradiance->Upload("uTexDeltaIrradiance"_hash, static_cast<int32>(m_TexDeltaMultipleScattering->GetHandle()));
		m_pComputeIndirectIrradiance->Upload("scattering_order"_hash, scatteringOrder);
		api->SetBlendEnabled({ false, true });
		RenderingSystems::Instance()->GetPrimitiveRenderer().Draw<primitives::Quad>();
		api->SetBlendEnabled(false);

		// Compute the multiple scattering, store it in
		// delta_multiple_scattering_texture, and accumulate it in
		// scattering_texture_.
		api->LinkTextureToFbo(0, m_TexDeltaMultipleScattering->GetHandle(), 0);
		api->LinkTextureToFbo(1, atmo->m_TexInscatter->GetHandle(), 0);
		api->SetDrawBufferCount(2);
		api->SetViewport(ivec2(0), m_Settings.m_ScatteringTexDim.xy);
		api->SetShader(m_pComputeMultipleScattering.get());
		m_pComputeMultipleScattering->Upload("luminance_from_radiance"_hash, luminanceFromRadiance);
		api->LazyBindTexture(atmo->m_TexTransmittance->GetHandle(), atmo->m_TexTransmittance->GetTargetType(), atmo->m_TexTransmittance->GetHandle());
		m_pComputeMultipleScattering->Upload("uTexTransmittance"_hash, static_cast<int32>(atmo->m_TexTransmittance->GetHandle()));
		api->LazyBindTexture(m_TexDeltaScattering->GetHandle(), m_TexDeltaScattering->GetTargetType(), m_TexDeltaScattering->GetHandle());
		m_pComputeMultipleScattering->Upload("uTexDeltaScatteringDensity"_hash, static_cast<int32>(m_TexDeltaScattering->GetHandle()));
		for (int32 layer = 0; layer < m_Settings.m_ScatteringTexDim.z; ++layer)
		{
			m_pComputeMultipleScattering->Upload("layer"_hash, layer);
			api->SetBlendEnabled({ false, true });
			RenderingSystems::Instance()->GetPrimitiveRenderer().Draw<primitives::Quad>();
			api->SetBlendEnabled(false);
		}
	}
	api->LinkTextureToFbo(1, 0, 0);
	api->LinkTextureToFbo(2, 0, 0);
	api->LinkTextureToFbo(3, 0, 0);

	api->SetBlendEnabled(false);

	Unload();
}

void AtmospherePrecompute::SetUniforms(ShaderData* shader, TextureData* transmittance, TextureData* scattering, TextureData* irradiance, TextureData* mie)
{
	I_GraphicsApiContext* const api = Viewport::GetCurrentApiContext();

	api->LazyBindTexture(transmittance->GetHandle(), transmittance->GetTargetType(), transmittance->GetHandle());
	shader->Upload("uTexTransmittance"_hash, static_cast<int32>(transmittance->GetHandle()));

	api->LazyBindTexture(scattering->GetHandle(), scattering->GetTargetType(), scattering->GetHandle());
	shader->Upload("uTexScattering"_hash, static_cast<int32>(scattering->GetHandle()));

	api->LazyBindTexture(irradiance->GetHandle(), irradiance->GetTargetType(), irradiance->GetHandle());
	shader->Upload("uTexIrradiance"_hash, static_cast<int32>(irradiance->GetHandle()));

	api->LazyBindTexture(mie->GetHandle(), mie->GetTargetType(), mie->GetHandle());
	shader->Upload("uTexMie"_hash, static_cast<int32>(mie->GetHandle()));
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

		color = color + (rgb*irradiance / solarRGB * etm::pow((double)lambda / lambdaVec, lambda_power));
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
