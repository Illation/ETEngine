#include "stdafx.hpp"
#include "AtmospherePrecompute.h"
#include "Atmosphere.hpp"
#include "PrimitiveRenderer.hpp"
#include "../Graphics/ShaderData.hpp"


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

	m_pComputeTransmittance = ContentManager::Load<ShaderData>("Shaders/AtmoPreComp/ComputeTransmittance.glsl");
	m_pComputeDirectIrradiance = ContentManager::Load<ShaderData>("Shaders/AtmoPreComp/ComputeDirectIrradiance.glsl");
	m_pComputeSingleScattering = ContentManager::Load<ShaderData>("Shaders/AtmoPreComp/ComputeSingleScattering.glsl");
	m_pComputeScatteringDensity = ContentManager::Load<ShaderData>("Shaders/AtmoPreComp/ComputeScatteringDensity.glsl");
	m_pComputeIndirectIrradiance = ContentManager::Load<ShaderData>("Shaders/AtmoPreComp/ComputeIndirectIrradiance.glsl");
	m_pComputeMultipleScattering = ContentManager::Load<ShaderData>("Shaders/AtmoPreComp/ComputeMultipleScattering.glsl");

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

	glGenFramebuffers(1, &m_FBO);

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

	glDeleteFramebuffers(1, &m_FBO);
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
	atmo->m_Params.Upload(m_pComputeTransmittance, "uAtmosphere");
	m_Settings.UploadTextureSize(m_pComputeTransmittance);
	atmo->m_Params.Upload(m_pComputeDirectIrradiance, "uAtmosphere");
	m_Settings.UploadTextureSize(m_pComputeDirectIrradiance);
	atmo->m_Params.Upload(m_pComputeSingleScattering, "uAtmosphere");
	m_Settings.UploadTextureSize(m_pComputeSingleScattering);
	atmo->m_Params.Upload(m_pComputeScatteringDensity, "uAtmosphere");
	m_Settings.UploadTextureSize(m_pComputeScatteringDensity);
	atmo->m_Params.Upload(m_pComputeIndirectIrradiance, "uAtmosphere");
	m_Settings.UploadTextureSize(m_pComputeIndirectIrradiance);
	atmo->m_Params.Upload(m_pComputeMultipleScattering, "uAtmosphere");
	m_Settings.UploadTextureSize(m_pComputeMultipleScattering);

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

	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, atmo->m_TexTransmittance->GetHandle(), 0);
	glDrawBuffer(GL_COLOR_ATTACHMENT0);
	STATE->SetViewport(ivec2(0), ivec2(m_Settings.TRANSMITTANCE_W, m_Settings.TRANSMITTANCE_H));
	STATE->SetShader(m_pComputeTransmittance);
	PrimitiveRenderer::GetInstance()->Draw<primitives::Quad>();

	// Compute the direct irradiance, store it in delta_irradiance_texture and,
	// depending on 'blend', either initialize irradiance_texture_ with zeros or
	// leave it unchanged (we don't want the direct irradiance in
	// irradiance_texture_, but only the irradiance from the sky).
	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, m_TexDeltaIrradiance->GetHandle(), 0);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, atmo->m_TexIrradiance->GetHandle(), 0);
	glDrawBuffers(2, kDrawBuffers);
	STATE->SetViewport(ivec2(0), ivec2(m_Settings.IRRADIANCE_W, m_Settings.IRRADIANCE_H));
	STATE->SetShader(m_pComputeDirectIrradiance);
	STATE->LazyBindTexture(atmo->m_TexTransmittance->GetHandle(), atmo->m_TexTransmittance->GetTarget(), atmo->m_TexTransmittance->GetHandle());
	glUniform1i(glGetUniformLocation(m_pComputeDirectIrradiance->GetProgram(), "uTexTransmittance"), atmo->m_TexTransmittance->GetHandle());
	STATE->SetBlendEnabled({ false, blend });
	PrimitiveRenderer::GetInstance()->Draw<primitives::Quad>();
	STATE->SetBlendEnabled(false);

	// Compute the rayleigh and mie single scattering, store them in
	// delta_rayleigh_scattering_texture and delta_mie_scattering_texture, and
	// either store them or accumulate them in scattering_texture_ and
	// optional_single_mie_scattering_texture_.
	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, m_TexDeltaRayleigh->GetHandle(), 0);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, m_TexDeltaMie->GetHandle(), 0);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, atmo->m_TexInscatter->GetHandle(), 0);
	//if (optional_single_mie_scattering_texture_ != 0)
	//{
	//	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, optional_single_mie_scattering_texture_, 0);
	//	glDrawBuffers(4, kDrawBuffers);
	//}
	//else
	//{
	glDrawBuffers(3, kDrawBuffers);
	//}
	STATE->SetViewport(ivec2(0), m_Settings.m_ScatteringTexDim.xy);
	STATE->SetViewport(ivec2(0), m_Settings.m_ScatteringTexDim.xy);
	STATE->SetShader(m_pComputeSingleScattering);
	glUniformMatrix3fv(glGetUniformLocation(m_pComputeSingleScattering->GetProgram(), "luminance_from_radiance"), 1, GL_FALSE, etm::valuePtr(luminanceFromRadiance));
	STATE->LazyBindTexture(atmo->m_TexTransmittance->GetHandle(), atmo->m_TexTransmittance->GetTarget(), atmo->m_TexTransmittance->GetHandle());
	glUniform1i(glGetUniformLocation(m_pComputeSingleScattering->GetProgram(), "uTexTransmittance"), atmo->m_TexTransmittance->GetHandle());
	for (int32 layer = 0; layer < m_Settings.m_ScatteringTexDim.z; ++layer)
	{
		glUniform1i(glGetUniformLocation(m_pComputeSingleScattering->GetProgram(), "layer"), layer);
		STATE->SetBlendEnabled({ false, false, blend, blend });
		PrimitiveRenderer::GetInstance()->Draw<primitives::Quad>();
		STATE->SetBlendEnabled(false);
	}

	// Compute the 2nd, 3rd and 4th order of scattering, in sequence.
	for (int32 scatteringOrder = 2; scatteringOrder <= numScatteringOrders; ++scatteringOrder)
	{
		// Compute the scattering density, and store it in
		// delta_scattering_density_texture.
		glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, m_TexDeltaScattering->GetHandle(), 0);
		glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, 0, 0);
		glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, 0, 0);
		glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, 0, 0);
		glDrawBuffer(GL_COLOR_ATTACHMENT0);
		STATE->SetViewport(ivec2(0), m_Settings.m_ScatteringTexDim.xy);
		STATE->SetShader(m_pComputeScatteringDensity);
		STATE->LazyBindTexture(atmo->m_TexTransmittance->GetHandle(), atmo->m_TexTransmittance->GetTarget(), atmo->m_TexTransmittance->GetHandle());
		glUniform1i(glGetUniformLocation(m_pComputeScatteringDensity->GetProgram(), "uTexTransmittance"), atmo->m_TexTransmittance->GetHandle());
		STATE->LazyBindTexture(m_TexDeltaRayleigh->GetHandle(), m_TexDeltaRayleigh->GetTarget(), m_TexDeltaRayleigh->GetHandle());
		glUniform1i(glGetUniformLocation(m_pComputeScatteringDensity->GetProgram(), "uTexRayleigh"), m_TexDeltaRayleigh->GetHandle());
		STATE->LazyBindTexture(m_TexDeltaMie->GetHandle(), m_TexDeltaMie->GetTarget(), m_TexDeltaMie->GetHandle());
		glUniform1i(glGetUniformLocation(m_pComputeScatteringDensity->GetProgram(), "uTexDeltaMie"), m_TexDeltaMie->GetHandle());
		STATE->LazyBindTexture(m_TexDeltaMultipleScattering->GetHandle(), m_TexDeltaMultipleScattering->GetTarget(), m_TexDeltaMultipleScattering->GetHandle());
		glUniform1i(glGetUniformLocation(m_pComputeScatteringDensity->GetProgram(), "uTexMultipleScattering"), m_TexDeltaMultipleScattering->GetHandle());
		STATE->LazyBindTexture(m_TexDeltaIrradiance->GetHandle(), m_TexDeltaIrradiance->GetTarget(), m_TexDeltaIrradiance->GetHandle());
		glUniform1i(glGetUniformLocation(m_pComputeScatteringDensity->GetProgram(), "uTexDeltaIrradiance"), m_TexDeltaIrradiance->GetHandle());
		glUniform1i(glGetUniformLocation(m_pComputeScatteringDensity->GetProgram(), "scattering_order"), scatteringOrder);
		for (int32 layer = 0; layer < m_Settings.m_ScatteringTexDim.z; ++layer)
		{
			glUniform1i(glGetUniformLocation(m_pComputeScatteringDensity->GetProgram(), "layer"), layer);
			PrimitiveRenderer::GetInstance()->Draw<primitives::Quad>();
		}

		// Compute the indirect irradiance, store it in delta_irradiance_texture and
		// accumulate it in irradiance_texture_.
		glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, m_TexDeltaIrradiance->GetHandle(), 0);
		glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, atmo->m_TexIrradiance->GetHandle(), 0);
		glDrawBuffers(2, kDrawBuffers);
		STATE->SetViewport(ivec2(0), ivec2(m_Settings.IRRADIANCE_W, m_Settings.IRRADIANCE_H));
		STATE->SetShader(m_pComputeIndirectIrradiance);
		glUniformMatrix3fv(glGetUniformLocation(m_pComputeIndirectIrradiance->GetProgram(), "luminance_from_radiance"), 1, GL_FALSE, etm::valuePtr(luminanceFromRadiance));
		STATE->LazyBindTexture(m_TexDeltaRayleigh->GetHandle(), m_TexDeltaRayleigh->GetTarget(), m_TexDeltaRayleigh->GetHandle());
		glUniform1i(glGetUniformLocation(m_pComputeIndirectIrradiance->GetProgram(), "uTexRayleigh"), m_TexDeltaRayleigh->GetHandle());
		STATE->LazyBindTexture(m_TexDeltaMie->GetHandle(), m_TexDeltaMie->GetTarget(), m_TexDeltaMie->GetHandle());
		glUniform1i(glGetUniformLocation(m_pComputeIndirectIrradiance->GetProgram(), "uTexDeltaMie"), m_TexDeltaMie->GetHandle());
		STATE->LazyBindTexture(m_TexDeltaMultipleScattering->GetHandle(), m_TexDeltaMultipleScattering->GetTarget(), m_TexDeltaMultipleScattering->GetHandle());
		glUniform1i(glGetUniformLocation(m_pComputeIndirectIrradiance->GetProgram(), "uTexMultipleScattering"), m_TexDeltaMultipleScattering->GetHandle());
		glUniform1i(glGetUniformLocation(m_pComputeIndirectIrradiance->GetProgram(), "scattering_order"), scatteringOrder);
		STATE->SetBlendEnabled({ false, true });
		PrimitiveRenderer::GetInstance()->Draw<primitives::Quad>();
		STATE->SetBlendEnabled(false);

		// Compute the multiple scattering, store it in
		// delta_multiple_scattering_texture, and accumulate it in
		// scattering_texture_.
		glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, m_TexDeltaMultipleScattering->GetHandle(), 0);
		glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, atmo->m_TexInscatter->GetHandle(), 0);
		glDrawBuffers(2, kDrawBuffers);
		STATE->SetViewport(ivec2(0), m_Settings.m_ScatteringTexDim.xy);
		STATE->SetShader(m_pComputeMultipleScattering);
		glUniformMatrix3fv(glGetUniformLocation(m_pComputeMultipleScattering->GetProgram(), "luminance_from_radiance"), 1, GL_FALSE, etm::valuePtr(luminanceFromRadiance));
		STATE->LazyBindTexture(atmo->m_TexTransmittance->GetHandle(), atmo->m_TexTransmittance->GetTarget(), atmo->m_TexTransmittance->GetHandle());
		glUniform1i(glGetUniformLocation(m_pComputeMultipleScattering->GetProgram(), "uTexTransmittance"), atmo->m_TexTransmittance->GetHandle());
		STATE->LazyBindTexture(m_TexDeltaScattering->GetHandle(), m_TexDeltaScattering->GetTarget(), m_TexDeltaScattering->GetHandle());
		glUniform1i(glGetUniformLocation(m_pComputeMultipleScattering->GetProgram(), "uTexDeltaScatteringDensity"), m_TexDeltaScattering->GetHandle());
		for (int32 layer = 0; layer < m_Settings.m_ScatteringTexDim.z; ++layer)
		{
			glUniform1i(glGetUniformLocation(m_pComputeMultipleScattering->GetProgram(), "layer"), layer);
			STATE->SetBlendEnabled({ false, true });
			PrimitiveRenderer::GetInstance()->Draw<primitives::Quad>();
			STATE->SetBlendEnabled(false);
		}
	}
	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, 0, 0);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, 0, 0);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, 0, 0);

	STATE->SetBlendEnabled(false);

	Unload();
}

void AtmospherePrecompute::SetUniforms(ShaderData* shader, TextureData* transmittance, TextureData* scattering, TextureData* irradiance, TextureData* mie)
{
	STATE->LazyBindTexture(transmittance->GetHandle(), transmittance->GetTarget(), transmittance->GetHandle());
	glUniform1i(glGetUniformLocation(shader->GetProgram(), "uTexTransmittance"), transmittance->GetHandle());

	STATE->LazyBindTexture(scattering->GetHandle(), scattering->GetTarget(), scattering->GetHandle());
	glUniform1i(glGetUniformLocation(shader->GetProgram(), "uTexScattering"), scattering->GetHandle());

	STATE->LazyBindTexture(irradiance->GetHandle(), irradiance->GetTarget(), irradiance->GetHandle());
	glUniform1i(glGetUniformLocation(shader->GetProgram(), "uTexIrradiance"), irradiance->GetHandle());

	STATE->LazyBindTexture(mie->GetHandle(), mie->GetTarget(), mie->GetHandle());
	glUniform1i(glGetUniformLocation(shader->GetProgram(), "uTexMie"), mie->GetHandle());
}

double AtmospherePrecompute::CieColorMatchingFunctionTableValue(const AtmosphereSettings &settings, double wavelength, int column)
{
	if (wavelength <= settings.kLambdaMin || wavelength >= settings.kLambdaMax)
	{
		return 0.0;
	}
	double u = (wavelength - settings.kLambdaMin) / 5.0;
	int row = static_cast<int>(std::floor(u));
	assert(row >= 0 && row + 1 < 95);
	assert(CIE_2_DEG_COLOR_MATCHING_FUNCTIONS[4 * row] <= wavelength && CIE_2_DEG_COLOR_MATCHING_FUNCTIONS[4 * (row + 1)] >= wavelength);
	u -= row;
	return CIE_2_DEG_COLOR_MATCHING_FUNCTIONS[4 * row + column] * (1.0 - u) + CIE_2_DEG_COLOR_MATCHING_FUNCTIONS[4 * (row + 1) + column] * u;
}

double AtmospherePrecompute::Interpolate(const std::vector<double>& wavelengths, const std::vector<double>& wavelength_function, double wavelength)
{
	assert(wavelength_function.size() == wavelengths.size());
	if (wavelength < wavelengths[0])
	{
		return wavelength_function[0];
	}
	for (unsigned int i = 0; i < wavelengths.size() - 1; ++i)
	{
		if (wavelength < wavelengths[i + 1])
		{
			double u = (wavelength - wavelengths[i]) / (wavelengths[i + 1] - wavelengths[i]);
			return wavelength_function[i] * (1.0 - u) + wavelength_function[i + 1] * u;
		}
	}
	return wavelength_function[wavelength_function.size() - 1];
}

void AtmospherePrecompute::ComputeSpectralRadianceToLuminanceFactors(const AtmosphereSettings &settings, const std::vector<double>& wavelengths,
	const std::vector<double>& solar_irradiance, double lambda_power, dvec3 &color)
{
	color = dvec3(0);
	double solar_r = Interpolate(wavelengths, solar_irradiance, settings.kLambdaR);
	double solar_g = Interpolate(wavelengths, solar_irradiance, settings.kLambdaG);
	double solar_b = Interpolate(wavelengths, solar_irradiance, settings.kLambdaB);
	int32 dlambda = 1;
	for (int lambda = settings.kLambdaMin; lambda < settings.kLambdaMax; lambda += dlambda)
	{
		double x_bar = CieColorMatchingFunctionTableValue(settings, lambda, 1);
		double y_bar = CieColorMatchingFunctionTableValue(settings, lambda, 2);
		double z_bar = CieColorMatchingFunctionTableValue(settings, lambda, 3);
		const double* xyz2srgb = XYZ_TO_SRGB;
		double r_bar = xyz2srgb[0] * x_bar + xyz2srgb[1] * y_bar + xyz2srgb[2] * z_bar;
		double g_bar = xyz2srgb[3] * x_bar + xyz2srgb[4] * y_bar + xyz2srgb[5] * z_bar;
		double b_bar = xyz2srgb[6] * x_bar + xyz2srgb[7] * y_bar + xyz2srgb[8] * z_bar;
		double irradiance = Interpolate(wavelengths, solar_irradiance, lambda);

		color.x += r_bar * irradiance / solar_r * pow(lambda / settings.kLambdaR, lambda_power);
		color.y += g_bar * irradiance / solar_g * pow(lambda / settings.kLambdaG, lambda_power);
		color.z += b_bar * irradiance / solar_b * pow(lambda / settings.kLambdaB, lambda_power);
	}
	color = color * (settings.MAX_LUMINOUS_EFFICACY * dlambda);
}

void AtmospherePrecompute::ConvertSpectrumToLinearSrgb( const std::vector<double>& wavelengths,
	const std::vector<double>& spectrum, double* r, double* g, double* b) 
{
	double x = 0.0;
	double y = 0.0;
	double z = 0.0;
	const int32 dlambda = 1;
	for (int lambda = m_Settings.kLambdaMin; lambda < m_Settings.kLambdaMax; lambda += dlambda) 
	{
		double value = Interpolate(wavelengths, spectrum, lambda);
		x += CieColorMatchingFunctionTableValue(m_Settings, lambda, 1) * value;
		y += CieColorMatchingFunctionTableValue(m_Settings, lambda, 2) * value;
		z += CieColorMatchingFunctionTableValue(m_Settings, lambda, 3) * value;
	}
	*r = m_Settings.MAX_LUMINOUS_EFFICACY * (XYZ_TO_SRGB[0] * x + XYZ_TO_SRGB[1] * y + XYZ_TO_SRGB[2] * z) * dlambda;
	*g = m_Settings.MAX_LUMINOUS_EFFICACY * (XYZ_TO_SRGB[3] * x + XYZ_TO_SRGB[4] * y + XYZ_TO_SRGB[5] * z) * dlambda;
	*b = m_Settings.MAX_LUMINOUS_EFFICACY * (XYZ_TO_SRGB[6] * x + XYZ_TO_SRGB[7] * y + XYZ_TO_SRGB[8] * z) * dlambda;
}