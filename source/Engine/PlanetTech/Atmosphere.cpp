#include "stdafx.hpp"
#include "Atmosphere.hpp"

#include "Planet.hpp"
#include "AtmospherePrecompute.h"
#include "../Framebuffers/Gbuffer.hpp"

#include "../Graphics/ShaderData.hpp"
#include "../SceneGraph/AbstractScene.hpp"
#include "../Components/LightComponent.hpp"
#include "../Graphics/TextureData.hpp"
#include "../GraphicsHelper/PrimitiveRenderer.hpp"
#include "../GraphicsHelper/RenderPipeline.hpp"
#include "../Graphics/Frustum.hpp"
#include "SpriteRenderer.hpp"
#include "Skybox.hpp"
#include "Light.hpp"

vec3 InterpolatedSpectrum(const std::vector<double_t> &wavelengths, const std::vector<double_t> &v, const dvec3 &lambdas, float scale)
{
	return vec3((float)AtmospherePrecompute::Interpolate(wavelengths, v, lambdas.x),
				(float)AtmospherePrecompute::Interpolate(wavelengths, v, lambdas.y),
				(float)AtmospherePrecompute::Interpolate(wavelengths, v, lambdas.z)) * scale;
}

Atmosphere::Atmosphere()
{
	// CALCULATE ATMOSPHERE PARAMETERS
	// *******************************
	constexpr int32 kLambdaMin = 360; // min wavelength
	constexpr int32 kLambdaMax = 830; // max wavelength
	constexpr double kSolarIrradiance[48] = 
	{
		1.11776, 1.14259, 1.01249, 1.14716, 1.72765, 1.73054, 1.6887, 1.61253,
		1.91198, 2.03474, 2.02042, 2.02212, 1.93377, 1.95809, 1.91686, 1.8298,
		1.8685, 1.8931, 1.85149, 1.8504, 1.8341, 1.8345, 1.8147, 1.78158, 1.7533,
		1.6965, 1.68194, 1.64654, 1.6048, 1.52143, 1.55622, 1.5113, 1.474, 1.4482,
		1.41018, 1.36775, 1.34188, 1.31429, 1.28303, 1.26758, 1.2367, 1.2082,
		1.18737, 1.14683, 1.12362, 1.1058, 1.07124, 1.04992
	};
	// Values from http://www.iup.uni-bremen.de/gruppen/molspec/databases/
	// referencespectra/o3spectra2011/index.html for 233K, summed and averaged in
	// each bin (e.g. the value for 360nm is the average of the original values
	// for all wavelengths between 360 and 370nm). Values in m^2.
	constexpr double kOzoneCrossSection[48] = 
	{
		1.18e-27, 2.182e-28, 2.818e-28, 6.636e-28, 1.527e-27, 2.763e-27, 5.52e-27,
		8.451e-27, 1.582e-26, 2.316e-26, 3.669e-26, 4.924e-26, 7.752e-26, 9.016e-26,
		1.48e-25, 1.602e-25, 2.139e-25, 2.755e-25, 3.091e-25, 3.5e-25, 4.266e-25,
		4.672e-25, 4.398e-25, 4.701e-25, 5.019e-25, 4.305e-25, 3.74e-25, 3.215e-25,
		2.662e-25, 2.238e-25, 1.852e-25, 1.473e-25, 1.209e-25, 9.423e-26, 7.455e-26,
		6.566e-26, 5.105e-26, 4.15e-26, 4.228e-26, 3.237e-26, 2.451e-26, 2.801e-26,
		2.534e-26, 1.624e-26, 1.465e-26, 2.078e-26, 1.383e-26, 7.105e-27
	};
	// From https://en.wikipedia.org/wiki/Dobson_unit, in molecules.m^-2.
	constexpr double kDobsonUnit = 2.687e20;
	// Maximum number density of ozone molecules, in m^-3 (computed so at to get
	// 300 Dobson units of ozone - for this we divide 300 DU by the integral of
	// the ozone density profile defined below, which is equal to 15km).
	constexpr double kMaxOzoneNumberDensity = 300.0 * kDobsonUnit / 15000.0;
	// Wavelength independent solar irradiance "spectrum" (not physically
	// realistic, but was used in the original implementation).
	constexpr double kRayleigh = 1.24062e-6;
	constexpr double kRayleighScaleHeight = 8000.0;
	constexpr double kMieScaleHeight = 1200.0;
	constexpr double kMieAngstromAlpha = 0.0;
	constexpr double kMieAngstromBeta = 5.328e-3;
	constexpr double kMieSingleScatteringAlbedo = 0.9;
	constexpr double kGroundAlbedo = 0.1;

	DensityProfileLayer rayleigh_layer(0.0, 1.0, -1.0 / kRayleighScaleHeight, 0.0, 0.0);
	DensityProfileLayer mie_layer(0.0, 1.0, -1.0 / kMieScaleHeight, 0.0, 0.0);
	// Density profile increasing linearly from 0 to 1 between 10 and 25km, and
	// decreasing linearly from 1 to 0 between 25 and 40km. This is an approximate
	// profile from http://www.kln.ac.lk/science/Chemistry/Teaching_Resources/
	// Documents/Introduction%20to%20atmospheric%20chemistry.pdf (page 10).
	std::vector<DensityProfileLayer> ozone_density; 
	ozone_density.push_back( DensityProfileLayer(25000.0, 0.0, 0.0, 1.0 / 15000.0, -2.0 / 3.0));
	ozone_density.push_back( DensityProfileLayer(0.0, 0.0, 0.0, -1.0 / 15000.0, 8.0 / 3.0));

	std::vector<double> wavelengths;
	std::vector<double> solar_irradiance;
	std::vector<double> rayleigh_scattering;
	std::vector<double> mie_scattering;
	std::vector<double> mie_extinction;
	std::vector<double> absorption_extinction;
	std::vector<double> ground_albedo;
	for (int32 l = kLambdaMin; l <= kLambdaMax; l += 10)
	{
		double lambda = static_cast<double>(l) * 1e-3;  // micro-meters
		double mie = kMieAngstromBeta / kMieScaleHeight * pow(lambda, -kMieAngstromAlpha);
		wavelengths.push_back(l);
		solar_irradiance.push_back(kSolarIrradiance[(l - kLambdaMin) / 10]);
		rayleigh_scattering.push_back(kRayleigh * pow(lambda, -4));
		mie_scattering.push_back(mie * kMieSingleScatteringAlbedo);
		mie_extinction.push_back(mie);
		absorption_extinction.push_back( kMaxOzoneNumberDensity * kOzoneCrossSection[(l - kLambdaMin) / 10] );
		ground_albedo.push_back(kGroundAlbedo);
	}

	//constexpr double kSunSolidAngle = etm::PI * kSunAngularRadius * kSunAngularRadius;

	AtmosphereSettings settings = AtmosphereSettings();
	dvec3 lambdas = dvec3(settings.kLambdaR, settings.kLambdaG, settings.kLambdaB);
	constexpr double kLengthUnitInMeters = 1000.0;

	m_Params = AtmosphereParameters();

	m_Params.solar_irradiance = InterpolatedSpectrum(wavelengths, solar_irradiance, lambdas, 1.f);
	m_Params.sun_angular_radius = (float)(0.00935 / 2.0);
	m_Params.bottom_radius = (float)(6360000.0 / kLengthUnitInMeters);
	m_Params.top_radius = (float)(6420000.0 / kLengthUnitInMeters);
	m_Params.bottom_radius = 1737.1f;// #temp , moon specific
	m_Params.top_radius = 1837.1f;// #temp , moon specific
	m_Params.rayleigh_density = DensityProfile({ rayleigh_layer }, (float)kLengthUnitInMeters);
	m_Params.rayleigh_scattering = InterpolatedSpectrum(wavelengths, rayleigh_scattering, lambdas, (float)kLengthUnitInMeters);
	m_Params.mie_density = DensityProfile({ mie_layer }, (float)kLengthUnitInMeters);
	m_Params.mie_scattering = InterpolatedSpectrum(wavelengths, mie_scattering, lambdas, (float)kLengthUnitInMeters);
	m_Params.mie_extinction = InterpolatedSpectrum(wavelengths, mie_extinction, lambdas, (float)kLengthUnitInMeters);
	m_Params.mie_phase_function_g = (float)0.8f;
	m_Params.absorption_density = DensityProfile(ozone_density, (float)kLengthUnitInMeters);
	m_Params.absorption_extinction = InterpolatedSpectrum(wavelengths, absorption_extinction, lambdas, (float)kLengthUnitInMeters);
	m_Params.ground_albedo = InterpolatedSpectrum(wavelengths, ground_albedo, lambdas, 1.f);
	m_Params.mu_s_min = cosf(etm::radians(102.f));

	AtmospherePrecompute::ComputeSpectralRadianceToLuminanceFactors(settings, wavelengths, solar_irradiance, -3, m_SkyColor);
	AtmospherePrecompute::ComputeSpectralRadianceToLuminanceFactors(settings, wavelengths, solar_irradiance, 0, m_SunColor);
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
	AtmospherePrecompute::GetInstance()->Precalculate( this );
}

void Atmosphere::Initialize()
{
	Precalculate();
	//Load and compile Shaders
	m_pShader = ContentManager::Load<ShaderData>("Shaders/PostAtmosphere.glsl");
	GetUniforms();
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

	//Hotreload shader
	if (INPUT->IsKeyboardKeyDown(SDL_SCANCODE_LALT) && INPUT->IsKeyboardKeyPressed('R'))
	{
		m_pShader = ContentManager::Reload<ShaderData>("Shaders/PostAtmosphere.glsl");
		GetUniforms();
	}
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

	m_Params.Upload(m_pShader, "uAtmosphere");
	AtmospherePrecompute::GetInstance()->GetSettings().UploadTextureSize(m_pShader);

	vec3 skySpectralRadToLum = vec3((float)m_SkyColor.x, (float)m_SkyColor.y, (float)m_SkyColor.z);
	glUniform3fv(m_uSkySpectralRadToLum, 1, etm::valuePtr(skySpectralRadToLum));
	vec3 sunSpectralRadToLum = vec3((float)m_SunColor.x, (float)m_SunColor.y, (float)m_SunColor.z);
	glUniform3fv(m_uSunSpectralRadToLum, 1, etm::valuePtr(sunSpectralRadToLum));

	glUniform1i(glGetUniformLocation(m_pShader->GetProgram(), "uTexInscatter"), 3);
	STATE->LazyBindTexture(3, GL_TEXTURE_2D, m_TexInscatter->GetHandle());
	glUniform1i(glGetUniformLocation(m_pShader->GetProgram(), "uTexIrridiance"), 4);
	STATE->LazyBindTexture(3, GL_TEXTURE_2D, m_TexIrradiance->GetHandle());
	glUniform1i(glGetUniformLocation(m_pShader->GetProgram(), "uTexTransmittance"), 5);
	STATE->LazyBindTexture(3, GL_TEXTURE_2D, m_TexTransmittance->GetHandle());

	glUniform3fv(m_uSunDir, 1, etm::valuePtr(m_pSun->GetTransform()->GetForward()));
	DirectionalLight* pDirLight = m_pSun->GetLight<DirectionalLight>();
	if (pDirLight) glUniform1f(m_uSunIntensity, pDirLight->GetBrightness());
	vec2 sunSize = vec2(tan(m_Params.sun_angular_radius), cos(m_Params.sun_angular_radius));
	glUniform2fv(m_uSunSize, 1, etm::valuePtr(sunSize));

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

	// #temp , hacking that visualization
	vec4 brightness = vec4(SCENE->GetPostProcessingSettings().exposure);
	float layer = SCENE->GetSkybox()->GetRoughness();
	SpriteRenderer::GetInstance()->Draw(m_TexInscatter, vec2(0, (float)(WINDOW.Height/2)), brightness, vec2(0), vec2(2), 0, pos.z, SpriteScalingMode::TEXTURE, layer);
	SpriteRenderer::GetInstance()->Draw(m_TexIrradiance, vec2(0, ((float)WINDOW.Height)*0.8f), brightness, vec2(0), vec2(8), 0, pos.z, SpriteScalingMode::TEXTURE);
	SpriteRenderer::GetInstance()->Draw(m_TexTransmittance, vec2(0, ((float)WINDOW.Height)*0.3f), brightness, vec2(0), vec2(2), 0, pos.z, SpriteScalingMode::TEXTURE);
}

void Atmosphere::GetUniforms()
{
	if (!m_pShader)
		return;

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

	m_uSunDir = glGetUniformLocation(m_pShader->GetProgram(), "SunDir");
	m_uSunIntensity = glGetUniformLocation(m_pShader->GetProgram(), "SunIntensity");
	m_uSunSize = glGetUniformLocation(m_pShader->GetProgram(), "uSunSize");

	m_uSkySpectralRadToLum = glGetUniformLocation(m_pShader->GetProgram(), "uSkySpectralRadToLum");
	m_uSunSpectralRadToLum = glGetUniformLocation(m_pShader->GetProgram(), "uSunSpectralRadToLum");
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
