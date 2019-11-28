#pragma once
#include <EtCore/Content/AssetPointer.h>

#include <EtRendering/PlanetTech/AtmosphereSettings.h>


class RenderingSystems;


namespace render {


class Atmosphere;


class AtmospherePrecompute final
{
public:
	//Separate from constructor so we can unload the resources if we don't need them anymore
	void Init();
	void Unload();

	void Precalculate(Atmosphere* atmo);
	void SetUniforms(ShaderData* shader, TextureData* transmittance,
		TextureData* scattering, TextureData* irradiance, TextureData* mie);

	static void ComputeSpectralRadianceToLuminanceFactors(const std::vector<double>& wavelengths, const std::vector<double>& solar_irradiance, double lambda_power, dvec3 &color);

	const AtmosphereSettings& GetSettings() { return m_Settings; }

private:
	void ConvertSpectrumToLinearSrgb(const std::vector<double>& wavelengths, const std::vector<double>& spectrum, dvec3 &rgb);

	friend class Atmosphere;// #temp

	//Textures - probably also need fbos
	TextureData* m_TexDeltaIrradiance;
	TextureData* m_TexDeltaRayleigh;
	TextureData* m_TexDeltaMultipleScattering;
	TextureData* m_TexDeltaMie;
	TextureData* m_TexDeltaScattering;

	T_FbLoc m_FBO;

	AtmosphereSettings m_Settings;

	//Precomputation Shaders
	AssetPtr<ShaderData> m_pComputeTransmittance;
	AssetPtr<ShaderData> m_pComputeDirectIrradiance;
	AssetPtr<ShaderData> m_pComputeSingleScattering;
	AssetPtr<ShaderData> m_pComputeScatteringDensity;
	AssetPtr<ShaderData> m_pComputeIndirectIrradiance;
	AssetPtr<ShaderData> m_pComputeMultipleScattering;

	//other
	int32 m_Step = 0;
	int32 m_Order = 0;

	bool m_IsInitialized = false;
	bool m_Finished = false;

private:
	friend class RenderingSystems;

	AtmospherePrecompute() = default;
	~AtmospherePrecompute();
};


} // namespace render
