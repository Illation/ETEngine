#pragma once
#include "../Graphics/FrameBuffer.hpp"
#include "../Graphics/TextureData.hpp"

#include "AtmosphereSettings.h"

class Planet;
class Atmosphere;

class AtmoPreComputer : public Singleton<AtmoPreComputer>
{
public:
	//Separate from constructor so we can unload the resources if we don't need them anymore
	void Init();
	void Unload();

	void Precalculate( Atmosphere* atmo );
	void SetUniforms(ShaderData* shader, TextureData* transmittance, 
		TextureData* scattering, TextureData* irradiance, TextureData* mie);

	void ComputeSpectralRadianceToLuminanceFactors(const std::vector<double>& wavelengths, const std::vector<double>& solar_irradiance, double lambda_power, double* k_r, double* k_g, double* k_b);
	void ConvertSpectrumToLinearSrgb(const std::vector<double>& wavelengths, const std::vector<double>& spectrum, double* r, double* g, double* b);
private:
	//Textures - probably also need fbos
	TextureData* m_TexDeltaIrradiance;
	TextureData* m_TexDeltaRayleigh;
	TextureData* m_TexDeltaMultipleScattering;
	TextureData* m_TexDeltaMie;
	TextureData* m_TexDeltaScattering;

	GLuint m_FBO;

	AtmosphereSettings m_Settings;

	//Precomputation Shaders
	ShaderData* m_pComputeTransmittance;
	ShaderData* m_pComputeDirectIrradiance;
	ShaderData* m_pComputeSingleScattering;
	ShaderData* m_pComputeScatteringDensity;
	ShaderData* m_pComputeIndirectIrradiance;
	ShaderData* m_pComputeMultipleScattering;

	//other
	int32 m_Step = 0;
	int32 m_Order = 0;

	bool m_IsInitialized = false;
	bool m_Finished = false;

private:
	friend class Singleton<AtmoPreComputer>;

	AtmoPreComputer();
	~AtmoPreComputer();
};

class Atmosphere
{
public:
	Atmosphere();
	~Atmosphere();

	void Precalculate();
	void Initialize();
	void Draw(Planet* pPlanet, float radius);

private:
	friend class AtmoPreComputer;

	//Camera and pos reconstruction from gbuffer
	GLint m_uMatModel;
	GLint m_uMatWVP;

	GLint m_uCamPos;
	GLint m_uProjA;
	GLint m_uProjB;
	GLint m_uViewProjInv;

	GLint m_uPosition;
	GLint m_uRadius;
	GLint m_uSurfaceRadius;

	//textures for precomputed data
	TextureData* m_TexTransmittance;
	TextureData* m_TexIrradiance;
	TextureData* m_TexInscatter;

	ShaderData* m_pShader;
};

