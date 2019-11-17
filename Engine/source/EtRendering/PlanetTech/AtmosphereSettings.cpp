#include "stdafx.h"
#include "AtmosphereSettings.h"

#include <EtCore/Content/ResourceManager.h>
#include <EtCore/Content/AssetStub.h>
#include <EtCore/FileSystem/Json/JsonParser.h>

#include <EtRendering/GlobalRenderingSystems/GlobalRenderingSystems.h>


namespace render {


vec3 InterpolatedSpectrum(const std::vector<double_t> &wavelengths, const std::vector<double_t> &v, const dvec3 &lambdas, float scale)
{
	dvec3 ret = RenderingSystems::Instance()->GetCie().Interpolate(wavelengths, v, lambdas);
	return vec3((float)ret.x, (float)ret.y, (float)ret.z) * scale;
}

DensityProfileLayer JSONDensityProfile(JSON::Object* jlayer)
{
	DensityProfileLayer ret = DensityProfileLayer();
	JSON::ApplyNumValue(jlayer, ret.width, "width");
	JSON::ApplyNumValue(jlayer, ret.exp_term, "exp term");
	JSON::ApplyNumValue(jlayer, ret.exp_scale, "exp scale");
	JSON::ApplyNumValue(jlayer, ret.linear_term, "linear term");
	JSON::ApplyNumValue(jlayer, ret.constant_term, "constant term");
	return ret;
}

AtmosphereParameters::AtmosphereParameters(T_Hash const assetId, dvec3 &skyColor, dvec3 &sunColor)
{
	AssetPtr<StubData> jsonText = ResourceManager::Instance()->GetAssetData<StubData>(assetId);

	JSON::Parser parser = JSON::Parser(std::string(jsonText->GetText(), jsonText->GetLength()));
	JSON::Object* root = parser.GetRoot();

	// CALCULATE ATMOSPHERE PARAMETERS
	// *******************************
	int32 kLambdaMin; JSON::ApplyNumValue(root, kLambdaMin, "lambdaMin"); // min wavelength
	int32 kLambdaMax; JSON::ApplyNumValue(root, kLambdaMax, "lambdaMax");// max wavelength
	auto kSolarIrradiance = (*root)["solarIrradiance"]->arr()->NumArr();
	auto kOzoneCrossSection = (*root)["ozoneCrossSection"]->arr()->NumArr();
	double kDobsonUnit; JSON::ApplyNumValue(root, kDobsonUnit, "dobsonUnit");
	double ozoneAltitude; JSON::ApplyNumValue(root, ozoneAltitude, "ozone altitude"); 
	double ozoneDUs; JSON::ApplyNumValue(root, ozoneDUs, "ozone dobson units"); 
	double kMaxOzoneNumberDensity = ozoneDUs * kDobsonUnit / ozoneAltitude;
	double kRayleigh; JSON::ApplyNumValue(root, kRayleigh, "rayleigh");
	double kRayleighScaleHeight; JSON::ApplyNumValue(root, kRayleighScaleHeight, "rayleighScaleHeight");
	double kMieScaleHeight; JSON::ApplyNumValue(root, kMieScaleHeight, "mieScaleHeight");
	double kMieAngstromAlpha; JSON::ApplyNumValue(root, kMieAngstromAlpha, "mieAngstromAlpha");
	double kMieAngstromBeta; JSON::ApplyNumValue(root, kMieAngstromBeta, "mieAngstromBeta");
	double kMieSingleScatteringAlbedo; JSON::ApplyNumValue(root, kMieSingleScatteringAlbedo, "mieSingleScatteringAlbedo");
	double kGroundAlbedo; JSON::ApplyNumValue(root, kGroundAlbedo, "groundAlbedo");

	DensityProfileLayer rayleigh_layer = JSONDensityProfile((*root)["rayleigh layer"]->obj()); rayleigh_layer.exp_scale /= (float)kRayleighScaleHeight;
	DensityProfileLayer mie_layer = JSONDensityProfile((*root)["mie layer"]->obj()); mie_layer.exp_scale /= (float)kMieScaleHeight;

	double ozoneConstDiv; JSON::ApplyNumValue(root, ozoneConstDiv, "ozone constant divisor");
	std::vector<DensityProfileLayer> ozone_density;
	JSON::Array* jOzoneDensity = (*root)["ozone density"]->arr();
	for (auto jlayer : jOzoneDensity->value)
	{
		DensityProfileLayer layer = JSONDensityProfile(jlayer->obj());
		layer.linear_term /= (float)ozoneAltitude;
		layer.constant_term /= (float)ozoneConstDiv;
		ozone_density.push_back(layer);
	}

	int32 lambdaIncrement; JSON::ApplyNumValue(root, lambdaIncrement, "lambda increment");
	double rayleighLambdaExp; JSON::ApplyNumValue(root, rayleighLambdaExp, "rayleigh lambda exp");

	std::vector<double> wavelengths;
	std::vector<double> solar_irradiance;
	std::vector<double> rayleigh_scattering;
	std::vector<double> mie_scattering;
	std::vector<double> mie_extinction;
	std::vector<double> absorption_extinction;
	std::vector<double> ground_albedo;
	for (int32 l = kLambdaMin; l <= kLambdaMax; l += lambdaIncrement)
	{
		double lambda = static_cast<double>(l) * 1e-3;  // micro-meters
		double mie = kMieAngstromBeta / kMieScaleHeight * pow(lambda, -kMieAngstromAlpha);
		wavelengths.push_back(l);
		solar_irradiance.push_back(kSolarIrradiance[(l - kLambdaMin) / lambdaIncrement]);
		rayleigh_scattering.push_back(kRayleigh * pow(lambda, rayleighLambdaExp));
		mie_scattering.push_back(mie * kMieSingleScatteringAlbedo);
		mie_extinction.push_back(mie);
		absorption_extinction.push_back(kMaxOzoneNumberDensity * kOzoneCrossSection[(l - kLambdaMin) / lambdaIncrement]);
		ground_albedo.push_back(kGroundAlbedo);
	}

	AtmosphereSettings settings = AtmosphereSettings();
	dvec3 lambdas = dvec3(settings.kLambdaR, settings.kLambdaG, settings.kLambdaB);
	double kLengthUnitInMeters; JSON::ApplyNumValue(root, kLengthUnitInMeters, "length unit in meters");

	solarIrradiance = InterpolatedSpectrum(wavelengths, solar_irradiance, lambdas, 1.f);
	JSON::ApplyNumValue(root, sun_angular_radius, "sun angular diameter"); sun_angular_radius /= 2.0;
	JSON::ApplyNumValue(root, bottom_radius, "bottom radius"); bottom_radius /= (float)kLengthUnitInMeters;
	JSON::ApplyNumValue(root, top_radius, "top radius"); top_radius /= (float)kLengthUnitInMeters;
	bottom_radius = 1737.1f;// #temp , moon specific
	top_radius = 1837.1f;// #temp , moon specific
	rayleigh_density = DensityProfile({ rayleigh_layer }, (float)kLengthUnitInMeters);
	rayleighScattering = InterpolatedSpectrum(wavelengths, rayleigh_scattering, lambdas, (float)kLengthUnitInMeters);
	mie_density = DensityProfile({ mie_layer }, (float)kLengthUnitInMeters);
	mieScattering = InterpolatedSpectrum(wavelengths, mie_scattering, lambdas, (float)kLengthUnitInMeters);
	mieExtinction = InterpolatedSpectrum(wavelengths, mie_extinction, lambdas, (float)kLengthUnitInMeters);
	mie_phase_function_g = (float)0.8f;
	JSON::ApplyNumValue(root, mie_phase_function_g, "mie phase function");
	absorption_density = DensityProfile(ozone_density, (float)kLengthUnitInMeters);
	absorptionExtinction = InterpolatedSpectrum(wavelengths, absorption_extinction, lambdas, (float)kLengthUnitInMeters);
	groundAlbedo = InterpolatedSpectrum(wavelengths, ground_albedo, lambdas, 1.f);
	JSON::ApplyNumValue(root, mu_s_min, "mu s min"); mu_s_min = cosf(etm::radians(mu_s_min));

	AtmospherePrecompute::ComputeSpectralRadianceToLuminanceFactors(wavelengths, solar_irradiance, -3, skyColor);
	AtmospherePrecompute::ComputeSpectralRadianceToLuminanceFactors(wavelengths, solar_irradiance, 0, sunColor);
}

void AtmosphereParameters::Upload(ShaderData const* const shader, const std::string &varName) const
{
	Viewport::GetCurrentApiContext()->SetShader(shader);

	shader->Upload(GetHash(varName + ".solar_irradiance"), solarIrradiance, false);
	shader->Upload(GetHash(varName + ".sun_angular_radius"), sun_angular_radius, false);
	shader->Upload(GetHash(varName + ".bottom_radius"), bottom_radius, false);
	shader->Upload(GetHash(varName + ".top_radius"), top_radius, false);
	UploadDensityProfile(shader, varName + ".rayleigh_density", rayleigh_density);
	shader->Upload(GetHash(varName + ".rayleigh_scattering"), rayleighScattering, false);
	UploadDensityProfile(shader, varName + ".mie_density", mie_density);
	shader->Upload(GetHash(varName + ".mie_scattering"), mieScattering, false);
	shader->Upload(GetHash(varName + ".mie_extinction"), mieExtinction, false);
	shader->Upload(GetHash(varName + ".mie_phase_function_g"), mie_phase_function_g, false);
	UploadDensityProfile(shader, varName + ".absorption_density", absorption_density);
	shader->Upload(GetHash(varName + ".absorption_extinction"), absorptionExtinction, false);
	shader->Upload(GetHash(varName + ".ground_albedo"), groundAlbedo, false);
	shader->Upload(GetHash(varName + ".mu_s_min"), mu_s_min, false);
}

void AtmosphereParameters::UploadDensityProfile(ShaderData const* const shader, const std::string &varName, const DensityProfile &profile) const
{
	for (uint32 i = 0; i < 2; ++i)
	{
		std::string idxVar = varName + ".layers[" + std::to_string(i) + "].";
		shader->Upload(GetHash(idxVar + "width"), profile.layers[i].width, false);
		shader->Upload(GetHash(idxVar + "exp_term"), profile.layers[i].exp_term, false);
		shader->Upload(GetHash(idxVar + "exp_scale"), profile.layers[i].exp_scale, false);
		shader->Upload(GetHash(idxVar + "linear_term"), profile.layers[i].linear_term, false);
		shader->Upload(GetHash(idxVar + "constant_term"), profile.layers[i].constant_term, false);
	}
}

DensityProfileLayer::DensityProfileLayer(double width, double exp_term, double exp_scale, double linear_term, double constant_term) 
	: width((float)width)
	, exp_term((float)exp_term)
	, exp_scale((float)exp_scale)
	, linear_term((float)linear_term)
	, constant_term((float)constant_term)
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

void AtmosphereSettings::UploadTextureSize(ShaderData const* const shader) const
{
	Viewport::GetCurrentApiContext()->SetShader(shader);
	shader->Upload("uTexTransmittanceW"_hash, TRANSMITTANCE_W, false);
	shader->Upload("uTexTransmittanceH"_hash, TRANSMITTANCE_H, false);

	shader->Upload("uTexScatteringNuSize"_hash, INSCATTER_NU, false);
	shader->Upload("uTexScatteringMuSize"_hash, INSCATTER_MU, false);
	shader->Upload("uTexScatteringMuSSize"_hash, INSCATTER_MU_S, false);
	shader->Upload("uTexScatteringRSize"_hash, INSCATTER_R, false);

	shader->Upload("uTexIrradianceW"_hash, IRRADIANCE_W, false);
	shader->Upload("uTexIrradianceH"_hash, IRRADIANCE_H, false);
}

AtmosphereSettings::AtmosphereSettings()
{
	//General texture format for atmosphere
	m_TexParams = TextureParameters(false);
	m_TexParams.wrapS = E_TextureWrapMode::ClampToEdge;
	m_TexParams.wrapT = E_TextureWrapMode::ClampToEdge;
	m_TexParams.wrapR = E_TextureWrapMode::ClampToEdge;

	m_ScatteringTexDim = ivec3(INSCATTER_NU * INSCATTER_MU_S, INSCATTER_MU, INSCATTER_R);
}


} // namespace render
