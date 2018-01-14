#pragma once
#include "../Graphics/ShaderData.hpp"
#include "../Graphics/TextureData.hpp"
//Values copied from https://ebruneton.github.io/precomputed_atmospheric_scattering/atmosphere/constants.h.html

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
	AtmosphereParameters();
	void Upload(ShaderData* shader, const std::string &varName);

	vec3 solar_irradiance;
	float sun_angular_radius;
	float bottom_radius;
	float top_radius;
	DensityProfile rayleigh_density;
	vec3 rayleigh_scattering;
	DensityProfile mie_density;
	vec3 mie_scattering;
	vec3 mie_extinction;
	float mie_phase_function_g;
	DensityProfile absorption_density;
	vec3 absorption_extinction;
	vec3 ground_albedo;
	float mu_s_min;

private:
	void UploadDensityProfile(ShaderData* shader, const std::string &varName, const DensityProfile &profile);
};

struct AtmosphereSettings
{
public:
	AtmosphereSettings();
	//Texture Settings
	ivec3 m_ScatteringTexDim;
	TextureParameters m_TexParams;
	static const GLenum FORMAT = GL_RGBA;
	static const GLenum INTERNAL2D = GL_RGBA32F;
	static const GLenum INTERNAL3D = GL_RGBA16F;//half precision

	void UploadTextureSize(ShaderData* shader) const;
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