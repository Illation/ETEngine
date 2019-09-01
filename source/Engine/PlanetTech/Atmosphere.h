#pragma once
#include "AtmosphereSettings.h"

#include <EtCore/Content/AssetPointer.h>

#include <Engine/Graphics/FrameBuffer.h>
#include <Engine/Graphics/TextureData.h>


class Planet;
class Atmosphere;
class AtmoPreCompute;
class LightComponent;

class Atmosphere
{
public:
	Atmosphere() {}
	Atmosphere(T_Hash const parameterAssetId);
	~Atmosphere();

	void Precalculate();
	void Initialize();
	void Draw(Planet* pPlanet, float radius);

	void SetSunlight(LightComponent* pLight) { m_pSun = pLight; }

private:
	friend class AtmospherePrecompute;

	void GetUniforms();

	LightComponent* m_pSun;

	AtmosphereParameters m_Params;
	dvec3 m_SkyColor;
	dvec3 m_SunColor;

	//textures for precomputed data
	TextureData* m_TexTransmittance;
	TextureData* m_TexIrradiance;
	TextureData* m_TexInscatter;

	AssetPtr<ShaderData> m_pShader;
};

