#pragma once
#include <EtCore/Content/AssetPointer.h>

#include <EtRendering/PlanetTech/AtmosphereSettings.h>


namespace et {
namespace render {


class Atmosphere;
class RenderingSystems;


class AtmospherePrecompute final
{
public:
	//Separate from constructor so we can unload the resources if we don't need them anymore
	void Init();
	void Unload();

	void Precalculate(Atmosphere* atmo);
	void SetUniforms(rhi::ShaderData* shader, rhi::TextureData* transmittance,
		rhi::TextureData* scattering, rhi::TextureData* irradiance, rhi::TextureData* mie);

	static void ComputeSpectralRadianceToLuminanceFactors(const std::vector<double>& wavelengths, const std::vector<double>& solar_irradiance, double lambda_power, dvec3 &color);

	const AtmosphereSettings& GetSettings() { return m_Settings; }

private:
	void ConvertSpectrumToLinearSrgb(const std::vector<double>& wavelengths, const std::vector<double>& spectrum, dvec3 &rgb);

	friend class Atmosphere;// #temp

	//Textures - probably also need fbos
	rhi::TextureData* m_TexDeltaIrradiance;
	rhi::TextureData* m_TexDeltaRayleigh;
	rhi::TextureData* m_TexDeltaMultipleScattering;
	rhi::TextureData* m_TexDeltaMie;
	rhi::TextureData* m_TexDeltaScattering;

	rhi::T_FbLoc m_FBO;

	AtmosphereSettings m_Settings;

	//Precomputation Shaders
	AssetPtr<rhi::ShaderData> m_pComputeTransmittance;
	AssetPtr<rhi::ShaderData> m_pComputeDirectIrradiance;
	AssetPtr<rhi::ShaderData> m_pComputeSingleScattering;
	AssetPtr<rhi::ShaderData> m_pComputeScatteringDensity;
	AssetPtr<rhi::ShaderData> m_pComputeIndirectIrradiance;
	AssetPtr<rhi::ShaderData> m_pComputeMultipleScattering;

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
} // namespace et
