#include "stdafx.hpp"
#include "Atmosphere.hpp"

#include "Planet.hpp"
#include "../Framebuffers/Gbuffer.hpp"

#include "../Graphics/ShaderData.hpp"
#include "../SceneGraph/AbstractScene.hpp"
#include "../Components/LightComponent.hpp"
#include "../Graphics/TextureData.hpp"
#include "../GraphicsHelper/PrimitiveRenderer.hpp"
#include "../GraphicsHelper/RenderPipeline.hpp"
#include "../Graphics/Frustum.hpp"

Atmosphere::Atmosphere()
{
}
Atmosphere::~Atmosphere()
{
	delete m_TexTransmittance;
	m_TexTransmittance = nullptr;
	delete m_TexIrradiance;
	m_TexIrradiance = nullptr;
	delete m_TexInscatter;
	m_TexInscatter = nullptr;
}

void Atmosphere::Precalculate()
{
	//Calculate look up textures here
	AtmoPreComputer::GetInstance()->Precalculate( this );
}

void Atmosphere::Initialize()
{
	Precalculate();
	//Load and compile Shaders
	m_pShader = ContentManager::Load<ShaderData>("Shaders/PostAtmosphere.glsl");
	STATE->SetShader(m_pShader);

	m_uMatModel = glGetUniformLocation(m_pShader->GetProgram(), "model");
	m_uMatWVP = glGetUniformLocation(m_pShader->GetProgram(), "worldViewProj");

	m_uCamPos = glGetUniformLocation(m_pShader->GetProgram(), "camPos");
	m_uProjA = glGetUniformLocation(m_pShader->GetProgram(), "projectionA");
	m_uProjB = glGetUniformLocation(m_pShader->GetProgram(), "projectionB");
	m_uViewProjInv = glGetUniformLocation(m_pShader->GetProgram(), "viewProjInv");

	m_uPosition = glGetUniformLocation(m_pShader->GetProgram(), "Position");
	m_uRadius = glGetUniformLocation(m_pShader->GetProgram(), "Radius");
	m_uSurfaceRadius = glGetUniformLocation(m_pShader->GetProgram(), "SurfaceRadius");
}
void Atmosphere::Draw(Planet* pPlanet, float radius)
{
	vec3 pos = pPlanet->GetTransform()->GetPosition();
	float surfaceRadius = pPlanet->GetRadius();
	radius += surfaceRadius;
	float icoRadius = radius / 0.996407747f;//scale up the sphere so the face center reaches the top of the atmosphere

	Sphere objSphere = Sphere(pos, radius);
	if (CAMERA->GetFrustum()->ContainsSphere(objSphere) == VolumeCheck::OUTSIDE)
		return;

	//mat4 World = etm::translate(pos)*etm::scale(vec3(icoRadius));
	mat4 World = etm::scale(vec3(icoRadius))*etm::translate(pos);

	STATE->SetShader(m_pShader);

	glUniformMatrix4fv(m_uMatModel, 1, GL_FALSE, etm::valuePtr(World));
	glUniformMatrix4fv(m_uMatWVP, 1, GL_FALSE, etm::valuePtr(CAMERA->GetViewProj()));

	// #todo: stop repeating this everywhere
	glUniform1i(glGetUniformLocation(m_pShader->GetProgram(), "texGBufferA"), 0);
	glUniform1i(glGetUniformLocation(m_pShader->GetProgram(), "texGBufferB"), 1);
	glUniform1i(glGetUniformLocation(m_pShader->GetProgram(), "texGBufferC"), 2);
	auto gbufferTex = RenderPipeline::GetInstance()->GetGBuffer()->GetTextures();
	for (size_t i = 0; i < gbufferTex.size(); i++)
	{
		STATE->LazyBindTexture(i, GL_TEXTURE_2D, gbufferTex[i]->GetHandle());
	}
	glUniform1f(m_uProjA, CAMERA->GetDepthProjA());
	glUniform1f(m_uProjB, CAMERA->GetDepthProjB());
	glUniformMatrix4fv(m_uViewProjInv, 1, GL_FALSE, etm::valuePtr(CAMERA->GetStatViewProjInv()));
	glUniform3fv(m_uCamPos, 1, etm::valuePtr(CAMERA->GetTransform()->GetPosition()));

	glUniform3fv(m_uPosition, 1, etm::valuePtr(pos));
	glUniform1f(m_uRadius, radius);
	glUniform1f(m_uSurfaceRadius, surfaceRadius);

	STATE->SetCullEnabled(true);
	STATE->SetFaceCullingMode(GL_FRONT);
	STATE->SetDepthEnabled(false);
	STATE->SetBlendEnabled(true);
	STATE->SetBlendEquation(GL_FUNC_ADD);
	STATE->SetBlendFunction(GL_ONE, GL_ONE);
	PrimitiveRenderer::GetInstance()->Draw<primitives::IcoSphere<3> >();
	STATE->SetFaceCullingMode(GL_BACK);
	STATE->SetBlendEnabled(false);
	STATE->SetDepthEnabled(true);
	STATE->SetCullEnabled(false);
}


AtmoPreComputer::AtmoPreComputer()
{
	Init();
}
AtmoPreComputer::~AtmoPreComputer()
{
	Unload();
}

void AtmoPreComputer::Init()
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
	STATE->BindFramebuffer(m_FBO);

	//Precompute(fbo, delta_irradiance_texture, delta_rayleigh_scattering_texture,
	//	delta_mie_scattering_texture, delta_scattering_density_texture,
	//	delta_multiple_scattering_texture, lambdas, luminance_from_radiance,
	//	false /* blend */, num_scattering_orders);


	m_IsInitialized = true;
}

void AtmoPreComputer::Unload()
{
	glDeleteFramebuffers(m_FBO, 0);

	delete m_TexDeltaIrradiance;
	m_TexDeltaIrradiance = nullptr;
	delete m_TexDeltaRayleigh;
	m_TexDeltaRayleigh = nullptr;
	m_TexDeltaMultipleScattering = nullptr;
	delete m_TexDeltaMie;
	m_TexDeltaMie = nullptr;
	delete m_TexDeltaScattering;
	m_TexDeltaScattering = nullptr;

	assert(glGetError == 0);

	m_IsInitialized = false;
}

void AtmoPreComputer::Precalculate( Atmosphere* atmo )
{
	if (!m_IsInitialized)
	{
		Init();
	}
	vec3 lambdas = vec3(m_Settings.kLambdaR, m_Settings.kLambdaG, m_Settings.kLambdaB);
	mat3 luminance_from_radiance = mat3();

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

	const GLuint kDrawBuffers[4] =
	{
		GL_COLOR_ATTACHMENT0,
		GL_COLOR_ATTACHMENT1,
		GL_COLOR_ATTACHMENT2,
		GL_COLOR_ATTACHMENT3
	};
	//glBlendEquationSeparate(GL_FUNC_ADD, GL_FUNC_ADD);
	STATE->SetBlendEquation(GL_FUNC_ADD);
	//glBlendFuncSeparate(GL_ONE, GL_ONE, GL_ONE, GL_ONE);
	STATE->SetBlendFunction(GL_ONE, GL_ONE);

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
	STATE->LazyBindTexture(GL_TEXTURE0 + atmo->m_TexTransmittance->GetHandle(), atmo->m_TexTransmittance->GetTarget(), atmo->m_TexTransmittance->GetHandle());
	glUniform1i(glGetUniformLocation(m_pComputeDirectIrradiance->GetProgram(), "uTexTransmittance"), atmo->m_TexTransmittance->GetHandle());
	//DrawQuad({false, blend}, full_screen_quad_vao_);
	PrimitiveRenderer::GetInstance()->Draw<primitives::Quad>();

	// Compute the rayleigh and mie single scattering, store them in
	// delta_rayleigh_scattering_texture and delta_mie_scattering_texture, and
	// either store them or accumulate them in scattering_texture_ and
	// optional_single_mie_scattering_texture_.
	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, m_TexDeltaRayleigh->GetHandle(), 0);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, m_TexDeltaMie->GetHandle(), 0);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, m_TexDeltaScattering->GetHandle(), 0);
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
	glUniformMatrix3fv(glGetUniformLocation(m_pComputeSingleScattering->GetProgram(), "luminance_from_radiance"), 1, GL_FALSE, etm::valuePtr(luminance_from_radiance));
	STATE->LazyBindTexture(GL_TEXTURE0 + atmo->m_TexTransmittance->GetHandle(), atmo->m_TexTransmittance->GetTarget(), atmo->m_TexTransmittance->GetHandle());
	glUniform1i(glGetUniformLocation(m_pComputeSingleScattering->GetProgram(), "uTexTransmittance"), atmo->m_TexTransmittance->GetHandle());
	for (unsigned int layer = 0; layer < m_Settings.m_ScatteringTexDim.z; ++layer)
	{
		glUniform1i(glGetUniformLocation(m_pComputeSingleScattering->GetProgram(), "layer"), layer);
		//DrawQuad({false, false, blend, blend}, full_screen_quad_vao_);
		PrimitiveRenderer::GetInstance()->Draw<primitives::Quad>();
	}
}

void AtmoPreComputer::SetUniforms(ShaderData* shader, TextureData* transmittance, TextureData* scattering, TextureData* irradiance, TextureData* mie)
{
	STATE->LazyBindTexture(GL_TEXTURE0 + transmittance->GetHandle(), transmittance->GetTarget(), transmittance->GetHandle());
	glUniform1i(glGetUniformLocation(shader->GetProgram(), "uTexTransmittance"), transmittance->GetHandle());

	STATE->LazyBindTexture(GL_TEXTURE0 + scattering->GetHandle(), scattering->GetTarget(), scattering->GetHandle());
	glUniform1i(glGetUniformLocation(shader->GetProgram(), "uTexScattering"), scattering->GetHandle());

	STATE->LazyBindTexture(GL_TEXTURE0 + irradiance->GetHandle(), irradiance->GetTarget(), irradiance->GetHandle());
	glUniform1i(glGetUniformLocation(shader->GetProgram(), "uTexIrradiance"), irradiance->GetHandle());

	STATE->LazyBindTexture(GL_TEXTURE0 + mie->GetHandle(), mie->GetTarget(), mie->GetHandle());
	glUniform1i(glGetUniformLocation(shader->GetProgram(), "uTexMie"), mie->GetHandle());
}

constexpr int kLambdaMin = 360;
constexpr int kLambdaMax = 830;

double CieColorMatchingFunctionTableValue(double wavelength, int column)
{
	if (wavelength <= kLambdaMin || wavelength >= kLambdaMax)
	{
		return 0.0;
	}
	double u = (wavelength - kLambdaMin) / 5.0;
	int row = static_cast<int>(std::floor(u));
	assert(row >= 0 && row + 1 < 95);
	assert(CIE_2_DEG_COLOR_MATCHING_FUNCTIONS[4 * row] <= wavelength &&
		CIE_2_DEG_COLOR_MATCHING_FUNCTIONS[4 * (row + 1)] >= wavelength);
	u -= row;
	return CIE_2_DEG_COLOR_MATCHING_FUNCTIONS[4 * row + column] * (1.0 - u) +
		CIE_2_DEG_COLOR_MATCHING_FUNCTIONS[4 * (row + 1) + column] * u;
}

double Interpolate( const std::vector<double>& wavelengths, const std::vector<double>& wavelength_function, double wavelength)
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

void AtmoPreComputer::ComputeSpectralRadianceToLuminanceFactors( const std::vector<double>& wavelengths,
	const std::vector<double>& solar_irradiance,
	double lambda_power, double* k_r, double* k_g, double* k_b)
{
	*k_r = 0.0;
	*k_g = 0.0;
	*k_b = 0.0;
	double solar_r = Interpolate(wavelengths, solar_irradiance, m_Settings.kLambdaR);
	double solar_g = Interpolate(wavelengths, solar_irradiance, m_Settings.kLambdaG);
	double solar_b = Interpolate(wavelengths, solar_irradiance, m_Settings.kLambdaB);
	int dlambda = 1;
	for (int lambda = kLambdaMin; lambda < kLambdaMax; lambda += dlambda)
	{
		double x_bar = CieColorMatchingFunctionTableValue(lambda, 1);
		double y_bar = CieColorMatchingFunctionTableValue(lambda, 2);
		double z_bar = CieColorMatchingFunctionTableValue(lambda, 3);
		const double* xyz2srgb = XYZ_TO_SRGB;
		double r_bar = xyz2srgb[0] * x_bar + xyz2srgb[1] * y_bar + xyz2srgb[2] * z_bar;
		double g_bar = xyz2srgb[3] * x_bar + xyz2srgb[4] * y_bar + xyz2srgb[5] * z_bar;
		double b_bar = xyz2srgb[6] * x_bar + xyz2srgb[7] * y_bar + xyz2srgb[8] * z_bar;
		double irradiance = Interpolate(wavelengths, solar_irradiance, lambda);

		*k_r += r_bar * irradiance / solar_r * pow(lambda / m_Settings.kLambdaR, lambda_power);
		*k_g += g_bar * irradiance / solar_g * pow(lambda / m_Settings.kLambdaG, lambda_power);
		*k_b += b_bar * irradiance / solar_b * pow(lambda / m_Settings.kLambdaB, lambda_power);
	}
	*k_r *= MAX_LUMINOUS_EFFICACY * dlambda;
	*k_g *= MAX_LUMINOUS_EFFICACY * dlambda;
	*k_b *= MAX_LUMINOUS_EFFICACY * dlambda;
}

void AtmoPreComputer::ConvertSpectrumToLinearSrgb(
	const std::vector<double>& wavelengths,
	const std::vector<double>& spectrum,
	double* r, double* g, double* b) {
	double x = 0.0;
	double y = 0.0;
	double z = 0.0;
	const int dlambda = 1;
	for (int lambda = kLambdaMin; lambda < kLambdaMax; lambda += dlambda) {
		double value = Interpolate(wavelengths, spectrum, lambda);
		x += CieColorMatchingFunctionTableValue(lambda, 1) * value;
		y += CieColorMatchingFunctionTableValue(lambda, 2) * value;
		z += CieColorMatchingFunctionTableValue(lambda, 3) * value;
	}
	*r = MAX_LUMINOUS_EFFICACY *
		(XYZ_TO_SRGB[0] * x + XYZ_TO_SRGB[1] * y + XYZ_TO_SRGB[2] * z) * dlambda;
	*g = MAX_LUMINOUS_EFFICACY *
		(XYZ_TO_SRGB[3] * x + XYZ_TO_SRGB[4] * y + XYZ_TO_SRGB[5] * z) * dlambda;
	*b = MAX_LUMINOUS_EFFICACY *
		(XYZ_TO_SRGB[6] * x + XYZ_TO_SRGB[7] * y + XYZ_TO_SRGB[8] * z) * dlambda;
}

AtmosphereSettings::AtmosphereSettings()
{
	//General texture format for atmosphere
	m_TexParams = TextureParameters();
	m_TexParams.wrapS = GL_CLAMP_TO_EDGE;
	m_TexParams.wrapT = GL_CLAMP_TO_EDGE;
	m_TexParams.wrapR = GL_CLAMP_TO_EDGE;

	m_ScatteringTexDim = ivec3( INSCATTER_NU * INSCATTER_MU_S, INSCATTER_MU, INSCATTER_R);
}
