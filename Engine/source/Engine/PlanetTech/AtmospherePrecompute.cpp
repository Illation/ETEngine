#include "stdafx.h"
#include "AtmospherePrecompute.h"

#include "Atmosphere.h"

#include <glad/glad.h>

#include <EtCore/Content/ResourceManager.h>

#include <Engine/GraphicsHelper/PrimitiveRenderer.h>
#include <Engine/Graphics/Shader.h>
#include <Engine/Graphics/CIE.h>


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
	m_TexDeltaIrradiance = new TextureData(m_Settings.IRRADIANCE_W, m_Settings.IRRADIANCE_H,
		m_Settings.INTERNAL2D, m_Settings.FORMAT, GL_FLOAT);
	m_TexDeltaIrradiance->Build();
	m_TexDeltaIrradiance->SetParameters(m_Settings.m_TexParams);

	m_TexDeltaRayleigh = new TextureData(m_Settings.m_ScatteringTexDim.x, m_Settings.m_ScatteringTexDim.y, m_Settings.INTERNAL3D, m_Settings.FORMAT, GL_FLOAT, m_Settings.m_ScatteringTexDim.z);
	m_TexDeltaRayleigh->Build();
	m_TexDeltaRayleigh->SetParameters(m_Settings.m_TexParams);
	m_TexDeltaMultipleScattering = m_TexDeltaRayleigh; //Multiple scattering and rayleigh share same gpu texture space

	m_TexDeltaMie = new TextureData(m_Settings.m_ScatteringTexDim.x, m_Settings.m_ScatteringTexDim.y, m_Settings.INTERNAL3D, m_Settings.FORMAT, GL_FLOAT, m_Settings.m_ScatteringTexDim.z);
	m_TexDeltaMie->Build();
	m_TexDeltaMie->SetParameters(m_Settings.m_TexParams);

	m_TexDeltaScattering = new TextureData(m_Settings.m_ScatteringTexDim.x, m_Settings.m_ScatteringTexDim.y, m_Settings.INTERNAL3D, m_Settings.FORMAT, GL_FLOAT, m_Settings.m_ScatteringTexDim.z);
	m_TexDeltaScattering->Build();
	m_TexDeltaScattering->SetParameters(m_Settings.m_TexParams);

	STATE->GenFramebuffers(1, &m_FBO);

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

	STATE->DeleteFramebuffers(1, &m_FBO);
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

	STATE->BindFramebuffer(m_FBO);

	int32 numScatteringOrders = 4;
	mat3 luminanceFromRadiance = mat3(); //Might not be needed as we dont precompute luminance
	bool blend = false; //Same here

	//Specific texture initialization
	atmo->m_TexTransmittance = new TextureData(m_Settings.TRANSMITTANCE_W, m_Settings.TRANSMITTANCE_H,
		m_Settings.INTERNAL2D, m_Settings.FORMAT, GL_FLOAT);
	atmo->m_TexTransmittance->Build();
	atmo->m_TexTransmittance->SetParameters(m_Settings.m_TexParams);
	atmo->m_TexIrradiance = new TextureData(m_Settings.IRRADIANCE_W, m_Settings.IRRADIANCE_H,
		m_Settings.INTERNAL2D, m_Settings.FORMAT, GL_FLOAT);
	atmo->m_TexIrradiance->Build();
	atmo->m_TexIrradiance->SetParameters(m_Settings.m_TexParams);

	atmo->m_TexInscatter = new TextureData(m_Settings.m_ScatteringTexDim.x, m_Settings.m_ScatteringTexDim.y,
		m_Settings.INTERNAL3D, m_Settings.FORMAT, GL_FLOAT, m_Settings.m_ScatteringTexDim.z);
	atmo->m_TexInscatter->Build();
	atmo->m_TexInscatter->SetParameters(m_Settings.m_TexParams);

	//Buffers for blending
	const GLuint kDrawBuffers[4] =
	{
		GL_COLOR_ATTACHMENT0,
		GL_COLOR_ATTACHMENT1,
		GL_COLOR_ATTACHMENT2,
		GL_COLOR_ATTACHMENT3
	};
	STATE->SetBlendEquation(GL_FUNC_ADD);
	STATE->SetBlendFunction(GL_ONE, GL_ONE);
	STATE->SetBlendEnabled(false);

	STATE->LinkTextureToFbo(0, atmo->m_TexTransmittance->GetHandle(), 0);
	glDrawBuffer(GL_COLOR_ATTACHMENT0);
	STATE->SetViewport(ivec2(0), ivec2(m_Settings.TRANSMITTANCE_W, m_Settings.TRANSMITTANCE_H));
	STATE->SetShader(m_pComputeTransmittance.get());
	PrimitiveRenderer::GetInstance()->Draw<primitives::Quad>();

	// Compute the direct irradiance, store it in delta_irradiance_texture and,
	// depending on 'blend', either initialize irradiance_texture_ with zeros or
	// leave it unchanged (we don't want the direct irradiance in
	// irradiance_texture_, but only the irradiance from the sky).
	STATE->LinkTextureToFbo(0, m_TexDeltaIrradiance->GetHandle(), 0);
	STATE->LinkTextureToFbo(1, atmo->m_TexIrradiance->GetHandle(), 0);
	glDrawBuffers(2, kDrawBuffers);
	STATE->SetViewport(ivec2(0), ivec2(m_Settings.IRRADIANCE_W, m_Settings.IRRADIANCE_H));
	STATE->SetShader(m_pComputeDirectIrradiance.get());
	STATE->LazyBindTexture(atmo->m_TexTransmittance->GetHandle(), atmo->m_TexTransmittance->GetTarget(), atmo->m_TexTransmittance->GetHandle());
	m_pComputeDirectIrradiance->Upload("uTexTransmittance"_hash, static_cast<int32>(atmo->m_TexTransmittance->GetHandle()));
	STATE->SetBlendEnabled({ false, blend });
	PrimitiveRenderer::GetInstance()->Draw<primitives::Quad>();
	STATE->SetBlendEnabled(false);

	// Compute the rayleigh and mie single scattering, store them in
	// delta_rayleigh_scattering_texture and delta_mie_scattering_texture, and
	// either store them or accumulate them in scattering_texture_ and
	// optional_single_mie_scattering_texture_.
	STATE->LinkTextureToFbo(0, m_TexDeltaRayleigh->GetHandle(), 0);
	STATE->LinkTextureToFbo(1, m_TexDeltaMie->GetHandle(), 0);
	STATE->LinkTextureToFbo(2, atmo->m_TexInscatter->GetHandle(), 0);
	glDrawBuffers(3, kDrawBuffers);
	STATE->SetViewport(ivec2(0), m_Settings.m_ScatteringTexDim.xy);
	STATE->SetViewport(ivec2(0), m_Settings.m_ScatteringTexDim.xy);
	STATE->SetShader(m_pComputeSingleScattering.get());
	m_pComputeSingleScattering->Upload("luminance_from_radiance"_hash, luminanceFromRadiance);
	STATE->LazyBindTexture(atmo->m_TexTransmittance->GetHandle(), atmo->m_TexTransmittance->GetTarget(), atmo->m_TexTransmittance->GetHandle());
	m_pComputeSingleScattering->Upload("uTexTransmittance"_hash, static_cast<int32>(atmo->m_TexTransmittance->GetHandle()));
	for (int32 layer = 0; layer < m_Settings.m_ScatteringTexDim.z; ++layer)
	{
		m_pComputeSingleScattering->Upload("layer"_hash, layer);
		STATE->SetBlendEnabled({ false, false, blend, blend });
		PrimitiveRenderer::GetInstance()->Draw<primitives::Quad>();
		STATE->SetBlendEnabled(false);
	}

	// Compute the 2nd, 3rd and 4th order of scattering, in sequence.
	for (int32 scatteringOrder = 2; scatteringOrder <= numScatteringOrders; ++scatteringOrder)
	{
		// Compute the scattering density, and store it in
		// delta_scattering_density_texture.
		STATE->LinkTextureToFbo(0, m_TexDeltaScattering->GetHandle(), 0);
		STATE->LinkTextureToFbo(1, 0, 0);
		STATE->LinkTextureToFbo(2, 0, 0);
		STATE->LinkTextureToFbo(3, 0, 0);
		glDrawBuffer(GL_COLOR_ATTACHMENT0);
		STATE->SetViewport(ivec2(0), m_Settings.m_ScatteringTexDim.xy);
		STATE->SetShader(m_pComputeScatteringDensity.get());
		STATE->LazyBindTexture(atmo->m_TexTransmittance->GetHandle(), atmo->m_TexTransmittance->GetTarget(), atmo->m_TexTransmittance->GetHandle());
		m_pComputeScatteringDensity->Upload("uTexTransmittance"_hash, static_cast<int32>(atmo->m_TexTransmittance->GetHandle()));
		STATE->LazyBindTexture(m_TexDeltaRayleigh->GetHandle(), m_TexDeltaRayleigh->GetTarget(), m_TexDeltaRayleigh->GetHandle());
		m_pComputeScatteringDensity->Upload("uTexRayleigh"_hash, static_cast<int32>(m_TexDeltaRayleigh->GetHandle()));
		STATE->LazyBindTexture(m_TexDeltaMie->GetHandle(), m_TexDeltaMie->GetTarget(), m_TexDeltaMie->GetHandle());
		m_pComputeScatteringDensity->Upload("uTexDeltaMie"_hash, static_cast<int32>(m_TexDeltaMie->GetHandle()));
		STATE->LazyBindTexture(m_TexDeltaMultipleScattering->GetHandle(), m_TexDeltaMultipleScattering->GetTarget(), m_TexDeltaMultipleScattering->GetHandle());
		m_pComputeScatteringDensity->Upload("uTexMultipleScattering"_hash, static_cast<int32>(m_TexDeltaMultipleScattering->GetHandle()));
		STATE->LazyBindTexture(m_TexDeltaIrradiance->GetHandle(), m_TexDeltaIrradiance->GetTarget(), m_TexDeltaIrradiance->GetHandle());
		m_pComputeScatteringDensity->Upload("uTexDeltaIrradiance"_hash, static_cast<int32>(m_TexDeltaIrradiance->GetHandle()));
		m_pComputeScatteringDensity->Upload("scattering_order"_hash, scatteringOrder);
		for (int32 layer = 0; layer < m_Settings.m_ScatteringTexDim.z; ++layer)
		{
			m_pComputeScatteringDensity->Upload("layer"_hash, layer);
			PrimitiveRenderer::GetInstance()->Draw<primitives::Quad>();
		}

		// Compute the indirect irradiance, store it in delta_irradiance_texture and
		// accumulate it in irradiance_texture_.
		STATE->LinkTextureToFbo(0, m_TexDeltaIrradiance->GetHandle(), 0);
		STATE->LinkTextureToFbo(1, atmo->m_TexIrradiance->GetHandle(), 0);
		glDrawBuffers(2, kDrawBuffers);
		STATE->SetViewport(ivec2(0), ivec2(m_Settings.IRRADIANCE_W, m_Settings.IRRADIANCE_H));
		STATE->SetShader(m_pComputeIndirectIrradiance.get());
		m_pComputeIndirectIrradiance->Upload("luminance_from_radiance"_hash, luminanceFromRadiance);
		STATE->LazyBindTexture(m_TexDeltaRayleigh->GetHandle(), m_TexDeltaRayleigh->GetTarget(), m_TexDeltaRayleigh->GetHandle());
		m_pComputeIndirectIrradiance->Upload("uTexRayleigh"_hash, static_cast<int32>(m_TexDeltaRayleigh->GetHandle()));
		STATE->LazyBindTexture(m_TexDeltaMie->GetHandle(), m_TexDeltaMie->GetTarget(), m_TexDeltaMie->GetHandle());
		m_pComputeIndirectIrradiance->Upload("uTexDeltaMie"_hash, static_cast<int32>(m_TexDeltaMie->GetHandle()));
		STATE->LazyBindTexture(m_TexDeltaMultipleScattering->GetHandle(), m_TexDeltaMultipleScattering->GetTarget(), m_TexDeltaMultipleScattering->GetHandle());
		//m_pComputeIndirectIrradiance->Upload("uTexDeltaIrradiance"_hash, static_cast<int32>(m_TexDeltaMultipleScattering->GetHandle()));
		m_pComputeIndirectIrradiance->Upload("scattering_order"_hash, scatteringOrder);
		STATE->SetBlendEnabled({ false, true });
		PrimitiveRenderer::GetInstance()->Draw<primitives::Quad>();
		STATE->SetBlendEnabled(false);

		// Compute the multiple scattering, store it in
		// delta_multiple_scattering_texture, and accumulate it in
		// scattering_texture_.
		STATE->LinkTextureToFbo(0, m_TexDeltaMultipleScattering->GetHandle(), 0);
		STATE->LinkTextureToFbo(1, atmo->m_TexInscatter->GetHandle(), 0);
		glDrawBuffers(2, kDrawBuffers);
		STATE->SetViewport(ivec2(0), m_Settings.m_ScatteringTexDim.xy);
		STATE->SetShader(m_pComputeMultipleScattering.get());
		m_pComputeMultipleScattering->Upload("luminance_from_radiance"_hash, luminanceFromRadiance);
		STATE->LazyBindTexture(atmo->m_TexTransmittance->GetHandle(), atmo->m_TexTransmittance->GetTarget(), atmo->m_TexTransmittance->GetHandle());
		m_pComputeMultipleScattering->Upload("uTexTransmittance"_hash, static_cast<int32>(atmo->m_TexTransmittance->GetHandle()));
		STATE->LazyBindTexture(m_TexDeltaScattering->GetHandle(), m_TexDeltaScattering->GetTarget(), m_TexDeltaScattering->GetHandle());
		m_pComputeMultipleScattering->Upload("uTexDeltaScatteringDensity"_hash, static_cast<int32>(m_TexDeltaScattering->GetHandle()));
		for (int32 layer = 0; layer < m_Settings.m_ScatteringTexDim.z; ++layer)
		{
			m_pComputeMultipleScattering->Upload("layer"_hash, layer);
			STATE->SetBlendEnabled({ false, true });
			PrimitiveRenderer::GetInstance()->Draw<primitives::Quad>();
			STATE->SetBlendEnabled(false);
		}
	}
	STATE->LinkTextureToFbo(1, 0, 0);
	STATE->LinkTextureToFbo(2, 0, 0);
	STATE->LinkTextureToFbo(3, 0, 0);

	STATE->SetBlendEnabled(false);

	Unload();
}

void AtmospherePrecompute::SetUniforms(ShaderData* shader, TextureData* transmittance, TextureData* scattering, TextureData* irradiance, TextureData* mie)
{
	STATE->LazyBindTexture(transmittance->GetHandle(), transmittance->GetTarget(), transmittance->GetHandle());
	shader->Upload("uTexTransmittance"_hash, static_cast<int32>(transmittance->GetHandle()));

	STATE->LazyBindTexture(scattering->GetHandle(), scattering->GetTarget(), scattering->GetHandle());
	shader->Upload("uTexScattering"_hash, static_cast<int32>(scattering->GetHandle()));

	STATE->LazyBindTexture(irradiance->GetHandle(), irradiance->GetTarget(), irradiance->GetHandle());
	shader->Upload("uTexIrradiance"_hash, static_cast<int32>(irradiance->GetHandle()));

	STATE->LazyBindTexture(mie->GetHandle(), mie->GetTarget(), mie->GetHandle());
	shader->Upload("uTexMie"_hash, static_cast<int32>(mie->GetHandle()));
}

void AtmospherePrecompute::ComputeSpectralRadianceToLuminanceFactors(const std::vector<double>& wavelengths,
	const std::vector<double>& solar_irradiance, double lambda_power, dvec3 &color)
{
	color = dvec3(0);
	auto pCIE = CIE::GetInstance();
	int32 dlambda = 1;
	dvec3 lambdaVec = dvec3(AtmosphereSettings::kLambdaR, AtmosphereSettings::kLambdaG, AtmosphereSettings::kLambdaB);
	dvec3 solarRGB = pCIE->Interpolate(wavelengths, solar_irradiance, lambdaVec);
	for (int lambda = AtmosphereSettings::kLambdaMin; lambda < AtmosphereSettings::kLambdaMax; lambda += dlambda)
	{
		double irradiance = pCIE->Interpolate(wavelengths, solar_irradiance, lambda);

		dvec3 xyz = pCIE->GetValue(lambda, AtmosphereSettings::kLambdaMin, AtmosphereSettings::kLambdaMax);
		dvec3 rgb = pCIE->GetRGB(xyz);

		color = color + (rgb*irradiance / solarRGB * etm::pow((double)lambda / lambdaVec, lambda_power));
	}
	color = color * (AtmosphereSettings::MAX_LUMINOUS_EFFICACY * dlambda);
}

void AtmospherePrecompute::ConvertSpectrumToLinearSrgb( const std::vector<double>& wavelengths,
	const std::vector<double>& spectrum, dvec3 &rgb) 
{
	dvec3 xyz = dvec3(0);
	auto pCIE = CIE::GetInstance();
	const int32 dlambda = 1;
	for (int lambda = m_Settings.kLambdaMin; lambda < m_Settings.kLambdaMax; lambda += dlambda) 
	{
		double value = pCIE->Interpolate(wavelengths, spectrum, lambda);
		xyz = xyz + pCIE->GetValue(lambda, AtmosphereSettings::kLambdaMin, AtmosphereSettings::kLambdaMax) * value;
	}
	rgb = pCIE->GetRGB(xyz);
}