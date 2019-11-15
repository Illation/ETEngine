#pragma once
#include <Engine/Graphics/Shader.h>
#include <Engine/Graphics/TextureData.h>


//Values copied from https://ebruneton.github.io/precomputed_atmospheric_scattering/atmosphere/constants.h.html


namespace render { 


struct DensityProfileLayer
{
	DensityProfileLayer();
	DensityProfileLayer(double width, double exp_term, double exp_scale, double linear_term, double constant_term);
	float width;
	float exp_term;
	float exp_scale;
	float linear_term;
	float constant_term;
};
struct DensityProfile
{
	DensityProfile();
	DensityProfile(std::vector<DensityProfileLayer> layers, float lengthUnitInMeters);
	DensityProfileLayer layers[2];
};
struct AtmosphereParameters
{
	AtmosphereParameters() {}
	AtmosphereParameters(T_Hash const assetId, dvec3 &skyColor, dvec3 &sunColor);
	void Upload(ShaderData const* const shader, const std::string &varName) const;

	vec3 solarIrradiance;
	float sun_angular_radius;
	float bottom_radius;
	float top_radius;
	DensityProfile rayleigh_density;
	vec3 rayleighScattering;
	DensityProfile mie_density;
	vec3 mieScattering;
	vec3 mieExtinction;
	float mie_phase_function_g;
	DensityProfile absorption_density;
	vec3 absorptionExtinction;
	vec3 groundAlbedo;
	float mu_s_min;

private:
	void UploadDensityProfile(ShaderData const* const shader, const std::string &varName, const DensityProfile &profile) const;
};

struct AtmosphereSettings
{
public:
	AtmosphereSettings();
	//Texture Settings
	ivec3 m_ScatteringTexDim;
	TextureParameters m_TexParams;
	static const E_ColorFormat FORMAT = E_ColorFormat::RGBA;
	static const E_ColorFormat INTERNAL2D = E_ColorFormat::RGBA32f;
	static const E_ColorFormat INTERNAL3D = E_ColorFormat::RGBA16f;//half precision

	void UploadTextureSize(ShaderData const* const shader) const;
	int32 TRANSMITTANCE_W = 256;
	int32 TRANSMITTANCE_H = 64;

	int32 IRRADIANCE_W = 64;
	int32 IRRADIANCE_H = 16;

	int32 INSCATTER_R = 32;
	int32 INSCATTER_MU = 128;
	int32 INSCATTER_MU_S = 32;
	int32 INSCATTER_NU = 8;

	static constexpr double kLambdaR = 680.0;
	static constexpr double kLambdaG = 550.0;
	static constexpr double kLambdaB = 440.0;

	static constexpr int32 kLambdaMin = 360;
	static constexpr int32 kLambdaMax = 830;

	// The conversion factor between watts and lumens.
	static constexpr double MAX_LUMINOUS_EFFICACY = 683.0;

	//Other
	uint32 NUM_PRECOMPUTED_WAVELENGTHS = 3; //we approximate luminance but dont precompute it
};


} // namespace render