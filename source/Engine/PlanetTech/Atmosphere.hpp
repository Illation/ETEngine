#pragma once
#include "../Graphics/FrameBuffer.hpp"
#include "../Graphics/TextureData.hpp"

#include "AtmosphereSettings.h"

class Planet;
class Atmosphere;
class AtmoPreCompute;
class LightComponent;

class Atmosphere
{
public:
	Atmosphere();
	~Atmosphere();

	void Precalculate();
	void Initialize();
	void Draw(Planet* pPlanet, float radius);

	void SetSunlight(LightComponent* pLight) { m_pSun = pLight; }

private:
	friend class AtmospherePrecompute;

	void GetUniforms();

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

	GLint m_uSunDir;
	GLint m_uSunIntensity;
	GLint m_uSunSize;
	LightComponent* m_pSun;

	GLint m_uSkySpectralRadToLum;
	GLint m_uSunSpectralRadToLum;

	AtmosphereParameters m_Params;
	dvec3 m_SkyColor;
	dvec3 m_SunColor;

	//textures for precomputed data
	TextureData* m_TexTransmittance;
	TextureData* m_TexIrradiance;
	TextureData* m_TexInscatter;

	ShaderData* m_pShader;
};

