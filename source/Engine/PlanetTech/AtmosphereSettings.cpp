#include "stdafx.hpp"
#include "AtmosphereSettings.h"

AtmosphereParameters::AtmosphereParameters()
{

}

void AtmosphereParameters::Upload(ShaderData* shader, const std::string &varName)
{
	STATE->SetShader(shader);

	glUniform3fv(glGetUniformLocation(shader->GetProgram(), (varName + ".solar_irradiance").c_str()), 1, etm::valuePtr(solar_irradiance));
	glUniform1f(glGetUniformLocation(shader->GetProgram(), (varName + ".sun_angular_radius").c_str()), sun_angular_radius);
	glUniform1f(glGetUniformLocation(shader->GetProgram(), (varName + ".bottom_radius").c_str()), bottom_radius);
	glUniform1f(glGetUniformLocation(shader->GetProgram(), (varName + ".top_radius").c_str()), top_radius);
	UploadDensityProfile(shader, varName + ".rayleigh_density", rayleigh_density);
	glUniform3fv(glGetUniformLocation(shader->GetProgram(), (varName + ".rayleigh_scattering").c_str()), 1, etm::valuePtr(rayleigh_scattering));
	UploadDensityProfile(shader, varName + ".mie_density", mie_density);
	glUniform3fv(glGetUniformLocation(shader->GetProgram(), (varName + ".mie_scattering").c_str()), 1, etm::valuePtr(mie_scattering));
	glUniform3fv(glGetUniformLocation(shader->GetProgram(), (varName + ".mie_extinction").c_str()), 1, etm::valuePtr(mie_extinction));
	glUniform1f(glGetUniformLocation(shader->GetProgram(), (varName + ".mie_phase_function_g").c_str()), mie_phase_function_g);
	UploadDensityProfile(shader, varName + ".absorption_density", absorption_density);
	glUniform3fv(glGetUniformLocation(shader->GetProgram(), (varName + ".absorption_extinction").c_str()), 1, etm::valuePtr(absorption_extinction));
	glUniform3fv(glGetUniformLocation(shader->GetProgram(), (varName + ".ground_albedo").c_str()), 1, etm::valuePtr(ground_albedo));
	glUniform1f(glGetUniformLocation(shader->GetProgram(), (varName + ".mu_s_min").c_str()), mu_s_min);
}

void AtmosphereParameters::UploadDensityProfile(ShaderData* shader, const std::string &varName, const DensityProfile &profile)
{
	for (uint32 i = 0; i < 2; ++i)
	{
		std::string idxVar = varName + ".layers[" + std::to_string(i) + "].";
		glUniform1f(glGetUniformLocation(shader->GetProgram(), (idxVar + "width").c_str()), profile.layers[i].width);
		glUniform1f(glGetUniformLocation(shader->GetProgram(), (idxVar + "exp_term").c_str()), profile.layers[i].exp_term);
		glUniform1f(glGetUniformLocation(shader->GetProgram(), (idxVar + "exp_scale").c_str()), profile.layers[i].exp_scale);
		glUniform1f(glGetUniformLocation(shader->GetProgram(), (idxVar + "linear_term").c_str()), profile.layers[i].linear_term);
		glUniform1f(glGetUniformLocation(shader->GetProgram(), (idxVar + "constant_term").c_str()), profile.layers[i].constant_term);
	}
}

DensityProfileLayer::DensityProfileLayer(double width, double exp_term, double exp_scale, double linear_term, double constant_term) 
	: width(width)
	, exp_term(exp_term)
	, exp_scale(exp_scale)
	, linear_term(linear_term)
	, constant_term(constant_term)
{
}

DensityProfileLayer::DensityProfileLayer()
	: width(0)
	, exp_term(0)
	, exp_scale(0)
	, linear_term(0)
	, constant_term(0)
{
}

DensityProfile::DensityProfile()
{

}

DensityProfile::DensityProfile(std::vector<DensityProfileLayer> inLayers, float lengthUnitInMeters)
{
	constexpr int kLayerCount = 2;
	while (inLayers.size() < kLayerCount)
	{
		inLayers.insert(inLayers.begin(), DensityProfileLayer());
	}
	for (auto &layer : inLayers)
	{
		layer.width /= lengthUnitInMeters;
		layer.exp_scale *= lengthUnitInMeters;
		layer.linear_term *= lengthUnitInMeters;
	}
	layers[0] = inLayers[0];
	layers[1] = inLayers[1];
}

void AtmosphereSettings::UploadTextureSize(ShaderData* shader)
{
	STATE->SetShader(shader);
	glUniform1i(glGetUniformLocation(shader->GetProgram(), "uTexTransmittanceW"), TRANSMITTANCE_W);
	glUniform1i(glGetUniformLocation(shader->GetProgram(), "uTexTransmittanceH"), TRANSMITTANCE_H);

	glUniform1i(glGetUniformLocation(shader->GetProgram(), "uTexScatteringNuSize"), INSCATTER_NU);
	glUniform1i(glGetUniformLocation(shader->GetProgram(), "uTexScatteringMuSize"), INSCATTER_MU);
	glUniform1i(glGetUniformLocation(shader->GetProgram(), "uTexScatteringMuSSize"), INSCATTER_MU_S);
	glUniform1i(glGetUniformLocation(shader->GetProgram(), "uTexScatteringRSize"), INSCATTER_R);

	glUniform1i(glGetUniformLocation(shader->GetProgram(), "uTexIrradianceW"), IRRADIANCE_W);
	glUniform1i(glGetUniformLocation(shader->GetProgram(), "uTexIrradianceH"), IRRADIANCE_H);
}
