uniform float SurfaceRadius;
uniform float Radius;
uniform int TRANSMITTANCE_INTEGRAL_SAMPLES = 5;

// Rayleigh
const float HR = 8.0;
const vec3 betaR = vec3(5.8e-3, 1.35e-2, 3.31e-2);
// Mie
const float HM = 1.2;
const vec3 betaMSca = vec3(4e-3);
const vec3 betaMEx = betaMSca / 0.9;
const float mieG = 0.8;

const float EPSILON_INSCATTER = 0.004f;

#define RES_R 32.0
#define RES_MU 128.0
#define RES_MU_S 32.0
#define RES_NU 8.0

bool intersectSphere(in vec3 viewDir, in vec3 position, in vec3 camPos, in float radius, inout float ffDist, inout float bfDist)
{
	// vector from ray origin to center of the sphere
	vec3 sphereDir = position-camPos;
	float spDotSp = dot(sphereDir,sphereDir);
	float radiusSq = radius*radius;
	float sDotV = dot(sphereDir,viewDir);
	float innerOffsetSq = radiusSq - (spDotSp - (sDotV * sDotV));
	float innerOffset = sqrt(innerOffsetSq);
	if(spDotSp <= radiusSq)
	{
		// ray origin inside sphere, hit is ensured
		bfDist = sDotV + innerOffset;
		return true;
	}
	else if(sDotV >= 0 && innerOffsetSq >= 0)
	{
		// ray starts outside in front of sphere but hits
		ffDist = sDotV - innerOffset;
		bfDist = sDotV + innerOffset;
		return true;
	}
	return false;
}	

//float intersectAtmosphere(float alt, float mu)
//{
//	float RL = Radius * 1.001;
//    float dout = -alt * mu + sqrt(alt * alt * (mu * mu - 1.0) + RL * RL);
//    float delta2 = alt * alt * (mu * mu - 1.0) + SurfaceRadius * SurfaceRadius;
//    if (delta2 >= 0.0) 
//	{
//        float din = -alt * mu - sqrt(delta2);
//        if (din >= 0.0) 
//		{
//            dout = min(dout, din);
//        }
//    }
//    return dout;
//}

//float densityOverPath(in float scaleHeight, in float alt, in float mu)
//{
//	// if ray below horizon return max density
//	float cosHorizon = -sqrt(1.0f - ((SurfaceRadius*SurfaceRadius)/(alt*alt)));
//	if(mu < cosHorizon)
//	return 1e9;
//	float totalDensity = 0.0f;
//	float dx = intersectAtmosphere(alt,mu) / float(TRANSMITTANCE_INTEGRAL_SAMPLES);
//	float y_j = exp(-(alt-SurfaceRadius)/scaleHeight);
//	for (int i = 1; i<=TRANSMITTANCE_INTEGRAL_SAMPLES; ++i)
//	{
//		float x_i = float(i)*dx;
//		float alt_i = sqrt(alt*alt + x_i*x_i + 2.0f*x_i*alt*mu);
//		float y_i = exp(-(alt_i-SurfaceRadius)/scaleHeight);
//		totalDensity += (y_j+y_i)/2.0f*dx;
//		y_j = y_i;
//	}

//	return totalDensity;
//}

//float opticalDepth(float H, float r, float mu, float d) 
//{
//    float a = sqrt((0.5/H)*r);
//    vec2 a01 = a*vec2(mu, mu + d / r);
//    vec2 a01s = sign(a01);
//    vec2 a01sq = a01*a01;
//    float x = a01s.y > a01s.x ? exp(a01sq.x) : 0.0;
//    vec2 y = a01s / (2.3193*abs(a01) + sqrt(1.52*a01sq + 4.0)) * vec2(1.0, exp(-d/H*(d/(2.0*r)+mu)));
//    return sqrt((6.2831*H)*r) * exp((SurfaceRadius-r)/H) * (x + dot(y, vec2(1.0, -1.0)));
//}

//vec3 analyticTransmittance(float r, float mu, float d) 
//{
//    return exp(- betaR * opticalDepth(HR, r, mu, d) - betaMEx * opticalDepth(HM, r, mu, d));
//}

////vec4 texture4D(sampler3D table, float r, float mu, float muS, float nu)
////{
////	float H = sqrt(Rt * Rt - Rg * Rg);
////	float rho = sqrt(r * r - Rg * Rg);
////#ifdef INSCATTER_NON_LINEAR
////	float rmu = r * mu;
////	float delta = rmu * rmu - r * r + Rg * Rg;
////	vec4 cst = rmu < 0.0 && delta > 0.0 ? vec4(1.0, 0.0, 0.0, 0.5 - 0.5 / float(RES_MU)) : vec4(-1.0, H * H, H, 0.5 + 0.5 / float(RES_MU));
////	float uR = 0.5 / float(RES_R) + rho / H * (1.0 - 1.0 / float(RES_R));
////	float uMu = cst.w + (rmu * cst.x + sqrt(delta + cst.y)) / (rho + cst.z) * (0.5 - 1.0 / float(RES_MU));
////	// paper formula
////	//float uMuS = 0.5 / float(RES_MU_S) + max((1.0 - exp(-3.0 * muS - 0.6)) / (1.0 - exp(-3.6)), 0.0) * (1.0 - 1.0 / float(RES_MU_S));
////	// better formula
////	float uMuS = 0.5 / float(RES_MU_S) + (atan(max(muS, -0.1975) * tan(1.26 * 1.1)) / 1.1 + (1.0 - 0.26)) * 0.5 * (1.0 - 1.0 / float(RES_MU_S));
////#else
////	float uR = 0.5 / float(RES_R) + rho / H * (1.0 - 1.0 / float(RES_R));
////	float uMu = 0.5 / float(RES_MU) + (mu + 1.0) / 2.0 * (1.0 - 1.0 / float(RES_MU));
////	float uMuS = 0.5 / float(RES_MU_S) + max(muS + 0.2, 0.0) / 1.2 * (1.0 - 1.0 / float(RES_MU_S));
////#endif
////	float lerp = (nu + 1.0) / 2.0 * (float(RES_NU) - 1.0);
////	float uNu = floor(lerp);
////	lerp = lerp - uNu;
////	return texture3D(table, vec3((uNu + uMuS) / float(RES_NU), uMu, uR)) * (1.0 - lerp) +
////		   texture3D(table, vec3((uNu + uMuS + 1.0) / float(RES_NU), uMu, uR)) * lerp;
////}
//vec4 texture4D(sampler3D table, float r, float mu, float muS, float nu)
//{
//   	float H = sqrt(Radius * Radius - SurfaceRadius * SurfaceRadius);
//   	float rho = sqrt(r * r - SurfaceRadius * SurfaceRadius);

//    float rmu = r * mu;
//    float delta = rmu * rmu - r * r + SurfaceRadius * SurfaceRadius;
//    vec4 cst = (rmu < 0.0 && delta > 0.0) ? vec4(1.0, 0.0, 0.0, 0.5 - 0.5 / RES_MU) : vec4(-1.0, H * H, H, 0.5 + 0.5 / RES_MU);
//    float uR = 0.5 / RES_R + rho / H * (1.0 - 1.0 / RES_R);
//    float uMu = cst.w + (rmu * cst.x + sqrt(delta + cst.y)) / (rho + cst.z) * (0.5 - 1.0 / float(RES_MU));
//    // paper formula
//    //float uMuS = 0.5 / RES_MU_S + max((1.0 - exp(-3.0 * muS - 0.6)) / (1.0 - exp(-3.6)), 0.0) * (1.0 - 1.0 / RES_MU_S);
//    // better formula
//    float uMuS = 0.5 / RES_MU_S + (atan(max(muS, -0.1975) * tan(1.26 * 1.1)) / 1.1 + (1.0 - 0.26)) * 0.5 * (1.0 - 1.0 / RES_MU_S);

//    float lep = (nu + 1.0) / 2.0 * (RES_NU - 1.0);
//    float uNu = floor(lep);
//    lep = lep - uNu;

//    return texture(table, vec3((uNu + uMuS) / RES_NU, uMu, uR)) * (1.0 - lep) 
//	+ texture(table, vec3((uNu + uMuS + 1.0) / RES_NU, uMu, uR)) * lep;
//}

//vec3 getMie(vec4 rayMie) 
//{	
//	// approximated single Mie scattering (cf. approximate Cm in paragraph "Angular precision")
//	// rayMie.rgb=C*, rayMie.w=Cm,r
//   	return rayMie.rgb * rayMie.w / max(rayMie.r, 1e-4) * (betaR.r / betaR);
//}

//float phaseFunctionR(float mu) 
//{
//	// Rayleigh phase function
//    return (3.0 / (16.0 * PI)) * (1.0 + mu * mu);
//}

//float phaseFunctionM(float mu) 
//{
//	// Mie phase function
//   	 return 1.5 * 1.0 / (4.0 * PI) * (1.0 - mieG*mieG) * pow(1.0 + (mieG*mieG) - 2.0*mieG*mu, -3.0/2.0) * (1.0 + mu * mu) / (2.0 + mieG*mieG);
//}

//vec3 transmittance(sampler2D texTransmittance, float r, float mu) 
//{
//	// transmittance(=transparency) of atmosphere for infinite ray (r,mu)
//	// (mu=cos(view zenith angle)), intersections with ground ignored
//   	float uR, uMu;
//    uR = sqrt((r - SurfaceRadius) / (Radius - SurfaceRadius));
//    uMu = atan((mu + 0.15) / (1.0 + 0.15) * tan(1.5)) / 1.5;
    
//    return texture(texTransmittance, vec2(uMu, uR)).rgb;
//}

//vec3 irradiance(sampler2D texIrradiance, float r, float muS)
//{
//	float uR = (r - SurfaceRadius) / (Radius - SurfaceRadius);
//	float uMuS = (muS + 0.2) / (1.0 + 0.2);

//	return texture(texIrradiance, vec2(uMuS, uR)).rgb;
//}

uniform int uTexTransmittanceW = 256;
uniform int uTexTransmittanceH = 64;

uniform int uTexScatteringNuSize = 8;
uniform int uTexScatteringMuSize = 128;
uniform int uTexScatteringMuSSize = 32;
uniform int uTexScatteringRSize = 32;

uniform int uTexIrradianceW = 64;
uniform int uTexIrradianceH = 16;

struct DensityProfileLayer 
{
	float width;
	float exp_term;
	float exp_scale;
	float linear_term;
	float constant_term;
};
struct DensityProfile 
{
	DensityProfileLayer layers[2];
};
struct AtmosphereParameters 
{
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
};

float ClampCosine(float mu) 
{
	return clamp(mu, float(-1.0), float(1.0));
}
float ClampDistance(float d) 
{
	return max(d, 0.0 * 1.0);
}
float ClampRadius(in AtmosphereParameters atmosphere, float r) 
{
	return clamp(r, atmosphere.bottom_radius, atmosphere.top_radius);
}
float SafeSqrt(float a) 
{
	return sqrt(max(a, 0.0 * 1.0 * 1.0));
}

//***********************
// TRANSMITTANCE
//***********************

//Transmittance calculation
float DistanceToTopAtmosphereBoundary(in AtmosphereParameters atmosphere, float r, float mu) 
{
	float discriminant = r * r * (mu * mu - 1.0) + atmosphere.top_radius * atmosphere.top_radius;
	return ClampDistance(-r * mu + SafeSqrt(discriminant));
}
float DistanceToBottomAtmosphereBoundary(in AtmosphereParameters atmosphere, float r, float mu) 
{
	float discriminant = r * r * (mu * mu - 1.0) + atmosphere.bottom_radius * atmosphere.bottom_radius;
	return ClampDistance(-r * mu - SafeSqrt(discriminant));
}
bool RayIntersectsGround(in AtmosphereParameters atmosphere, float r, float mu) 
{
	return mu < 0.0 && r * r * (mu * mu - 1.0) + atmosphere.bottom_radius * atmosphere.bottom_radius >= 0.0 * 1.0;
}
float GetLayerDensity(in DensityProfileLayer layer, float altitude) 
{
	float density = layer.exp_term * exp(layer.exp_scale * altitude) +
		layer.linear_term * altitude + layer.constant_term;
	return clamp(density, float(0.0), float(1.0));
}
float GetProfileDensity(in DensityProfile profile, float altitude) 
{
	return altitude < profile.layers[0].width ?
		GetLayerDensity(profile.layers[0], altitude) :
		GetLayerDensity(profile.layers[1], altitude);
}
float ComputeOpticalLengthToTopAtmosphereBoundary( in AtmosphereParameters atmosphere, in DensityProfile profile, float r, float mu) 
{
	// Number of intervals for the numerical integration.
	const int SAMPLE_COUNT = 500;
	// The integration step, i.e. the length of each integration interval.
	float dx = DistanceToTopAtmosphereBoundary(atmosphere, r, mu) / float(SAMPLE_COUNT);
	// Integration loop.
	float result = 0.0 * 1.0;
	for (int i = 0; i <= SAMPLE_COUNT; ++i) 
	{
		float d_i = float(i) * dx;
		// Distance between the current sample point and the planet center.
		float r_i = sqrt(d_i * d_i + 2.0 * r * mu * d_i + r * r);
		// Number density at the current sample point (divided by the number density
		// at the bottom of the atmosphere, yielding a dimensionless number).
		float y_i = GetProfileDensity(profile, r_i - atmosphere.bottom_radius);
		// Sample weight (from the trapezoidal rule).
		float weight_i = i == 0 || i == SAMPLE_COUNT ? 0.5 : 1.0;
		result += y_i * weight_i * dx;
	}
	return result;
}
vec3 ComputeTransmittanceToTopAtmosphereBoundary( in AtmosphereParameters atmosphere, float r, float mu) 
{
	float rayleighOpt = ComputeOpticalLengthToTopAtmosphereBoundary( atmosphere, atmosphere.rayleigh_density, r, mu);
	float mieOpt = ComputeOpticalLengthToTopAtmosphereBoundary( atmosphere, atmosphere.mie_density, r, mu);
	float absorptionOpt = ComputeOpticalLengthToTopAtmosphereBoundary( atmosphere, atmosphere.absorption_density, r, mu);

	return exp(-( atmosphere.rayleigh_scattering * rayleighOpt 
		+ atmosphere.mie_extinction * mieOpt 
		+ atmosphere.absorption_extinction * absorptionOpt ));
}

//Transmittance precalculation
float GetTextureCoordFromUnitRange(float x, int texture_size) 
{
	return 0.5 / float(texture_size) + x * (1.0 - 1.0 / float(texture_size));
}
float GetUnitRangeFromTextureCoord(float u, int texture_size) 
{
	return (u - 0.5 / float(texture_size)) / (1.0 - 1.0 / float(texture_size));
}
vec2 GetTransmittanceTextureUvFromRMu(in AtmosphereParameters atmosphere, float r, float mu) 
{
	//assert(r >= atmosphere.bottom_radius && r <= atmosphere.top_radius);
	//assert(mu >= -1.0 && mu <= 1.0);
	// Distance to top atmosphere boundary for a horizontal ray at ground level.
	float H = sqrt(atmosphere.top_radius * atmosphere.top_radius - atmosphere.bottom_radius * atmosphere.bottom_radius);
	// Distance to the horizon.
	float rho = SafeSqrt(r * r - atmosphere.bottom_radius * atmosphere.bottom_radius);
	// Distance to the top atmosphere boundary for the ray (r,mu), and its minimum
	// and maximum values over all mu - obtained for (r,1) and (r,mu_horizon).
	float d = DistanceToTopAtmosphereBoundary(atmosphere, r, mu);
	float d_min = atmosphere.top_radius - r;
	float d_max = rho + H;
	float x_mu = (d - d_min) / (d_max - d_min);
	float x_r = rho / H;
	return vec2(GetTextureCoordFromUnitRange(x_mu, uTexTransmittanceW), GetTextureCoordFromUnitRange(x_r, uTexTransmittanceH));
}

//Transmittance lookup
vec3 GetTransmittanceToTopAtmosphereBoundary( in AtmosphereParameters atmosphere, in sampler2D transmittance_texture, float r, float mu) 
{
	vec2 uv = GetTransmittanceTextureUvFromRMu(atmosphere, r, mu);
	return vec3(texture(transmittance_texture, uv));
}
vec3 GetTransmittance( in AtmosphereParameters atmosphere, in sampler2D transmittance_texture, float r, float mu, float d, bool ray_r_mu_intersects_ground) 
{
	float r_d = ClampRadius(atmosphere, sqrt(d * d + 2.0 * r * mu * d + r * r));
	float mu_d = ClampCosine((r * mu + d) / r_d);

	if (ray_r_mu_intersects_ground) 
	{
		return min(
			GetTransmittanceToTopAtmosphereBoundary( atmosphere, transmittance_texture, r_d, -mu_d) /
			GetTransmittanceToTopAtmosphereBoundary( atmosphere, transmittance_texture, r, -mu),
			vec3(1.0));
	} 
	else 
	{
		return min(
			GetTransmittanceToTopAtmosphereBoundary( atmosphere, transmittance_texture, r, mu) /
			GetTransmittanceToTopAtmosphereBoundary( atmosphere, transmittance_texture, r_d, mu_d),
			vec3(1.0));
	}
}
vec3 GetTransmittanceToSun( in AtmosphereParameters atmosphere, in sampler2D transmittance_texture, float r, float mu_s) 
{
	float sin_theta_h = atmosphere.bottom_radius / r;
	float cos_theta_h = -sqrt(max(1.0 - sin_theta_h * sin_theta_h, 0.0));
	return GetTransmittanceToTopAtmosphereBoundary( atmosphere, transmittance_texture, r, mu_s) *
		smoothstep(-sin_theta_h * atmosphere.sun_angular_radius / 1.0, sin_theta_h * atmosphere.sun_angular_radius / 1.0, mu_s - cos_theta_h);
}

//***********************
// SINGLE SCATTERING
//***********************

// single scattering calculation

void ComputeSingleScatteringIntegrand(
    in AtmosphereParameters atmosphere, in sampler2D transmittance_texture,
    float r, float mu, float mu_s, float nu, float d, bool ray_r_mu_intersects_ground,
    out vec3 rayleigh, out vec3 mie) 
{
	float r_d = ClampRadius(atmosphere, sqrt(d * d + 2.0 * r * mu * d + r * r));
	float mu_s_d = ClampCosine((r * mu_s + d * nu) / r_d);
	vec3 transmittance = GetTransmittance( atmosphere, transmittance_texture, r, mu, d, ray_r_mu_intersects_ground) *
		GetTransmittanceToSun( atmosphere, transmittance_texture, r_d, mu_s_d);
	rayleigh = transmittance * GetProfileDensity( atmosphere.rayleigh_density, r_d - atmosphere.bottom_radius);
	mie = transmittance * GetProfileDensity( atmosphere.mie_density, r_d - atmosphere.bottom_radius);
}
float DistanceToNearestAtmosphereBoundary(in AtmosphereParameters atmosphere, float r, float mu, bool ray_r_mu_intersects_ground) 
{
	if (ray_r_mu_intersects_ground) 
	{
		return DistanceToBottomAtmosphereBoundary(atmosphere, r, mu);
	} 
	else 
	{
		return DistanceToTopAtmosphereBoundary(atmosphere, r, mu);
	}
}
void ComputeSingleScattering(
    in AtmosphereParameters atmosphere, in sampler2D transmittance_texture,
    float r, float mu, float mu_s, float nu, bool ray_r_mu_intersects_ground,
    out vec3 rayleigh, out vec3 mie) 
{
	// Number of intervals for the numerical integration.
	const int SAMPLE_COUNT = 50;
	// The integration step, i.e. the length of each integration interval.
	float dx = DistanceToNearestAtmosphereBoundary(atmosphere, r, mu, ray_r_mu_intersects_ground) / float(SAMPLE_COUNT);
	// Integration loop.
	vec3 rayleigh_sum = vec3(0.0);
	vec3 mie_sum = vec3(0.0);
	for (int i = 0; i <= SAMPLE_COUNT; ++i) 
	{
		float d_i = float(i) * dx;
		// The Rayleigh and Mie single scattering at the current sample point.
		vec3 rayleigh_i;
		vec3 mie_i;
		ComputeSingleScatteringIntegrand(atmosphere, transmittance_texture, r, mu, mu_s, nu, 
			d_i, ray_r_mu_intersects_ground, rayleigh_i, mie_i);
		// Sample weight (from the trapezoidal rule).
		float weight_i = (i == 0 || i == SAMPLE_COUNT) ? 0.5 : 1.0;
		rayleigh_sum += rayleigh_i * weight_i;
		mie_sum += mie_i * weight_i;
	}
	rayleigh = rayleigh_sum * dx * atmosphere.solar_irradiance *
	atmosphere.rayleigh_scattering;
	mie = mie_sum * dx * atmosphere.solar_irradiance * atmosphere.mie_scattering;
}
float RayleighPhaseFunction(float nu) 
{
	float k = 3.0 / (16.0 * PI * STERADIAN);
	return k * (1.0 + nu * nu);
}
float MiePhaseFunction(float g, float nu) 
{
	float k = 3.0 / (8.0 * PI * STERADIAN) * (1.0 - g * g) / (2.0 + g * g);
	return k * (1.0 + nu * nu) / pow(1.0 + g * g - 2.0 * g * nu, 1.5);
}

vec4 GetScatteringTextureUvwzFromRMuMuSNu(in AtmosphereParameters atmosphere,
	float r, float mu, float mu_s, float nu, bool ray_r_mu_intersects_ground) 
{
	// Distance to top atmosphere boundary for a horizontal ray at ground level.
	float H = sqrt(atmosphere.top_radius * atmosphere.top_radius - atmosphere.bottom_radius * atmosphere.bottom_radius);
	// Distance to the horizon.
	float rho = SafeSqrt(r * r - atmosphere.bottom_radius * atmosphere.bottom_radius);
	float u_r = GetTextureCoordFromUnitRange(rho / H, uTexScatteringRSize);

	// Discriminant of the quadratic equation for the intersections of the ray
	// (r,mu) with the ground (see RayIntersectsGround).
	float r_mu = r * mu;
	float discriminant = r_mu * r_mu - r * r + atmosphere.bottom_radius * atmosphere.bottom_radius;
	float u_mu;
	if (ray_r_mu_intersects_ground) 
	{
		// Distance to the ground for the ray (r,mu), and its minimum and maximum
		// values over all mu - obtained for (r,-1) and (r,mu_horizon).
		float d = -r_mu - SafeSqrt(discriminant);
		float d_min = r - atmosphere.bottom_radius;
		float d_max = rho;
		u_mu = 0.5 - 0.5 * GetTextureCoordFromUnitRange(d_max == d_min ? 0.0 :
			(d - d_min) / (d_max - d_min), uTexScatteringMuSize / 2);
	} 
	else 
	{
		// Distance to the top atmosphere boundary for the ray (r,mu), and its
		// minimum and maximum values over all mu - obtained for (r,1) and
		// (r,mu_horizon).
		float d = -r_mu + SafeSqrt(discriminant + H * H);
		float d_min = atmosphere.top_radius - r;
		float d_max = rho + H;
		u_mu = 0.5 + 0.5 * GetTextureCoordFromUnitRange( (d - d_min) / (d_max - d_min), uTexScatteringMuSize / 2);
	}

	float d = DistanceToTopAtmosphereBoundary( atmosphere, atmosphere.bottom_radius, mu_s);
	float d_min = atmosphere.top_radius - atmosphere.bottom_radius;
	float d_max = H;
	float a = (d - d_min) / (d_max - d_min);
	float A = -2.0 * atmosphere.mu_s_min * atmosphere.bottom_radius / (d_max - d_min);
	float u_mu_s = GetTextureCoordFromUnitRange( max(1.0 - a / A, 0.0) / (1.0 + a), uTexScatteringMuSSize);

	float u_nu = (nu + 1.0) / 2.0;
	return vec4(u_nu, u_mu_s, u_mu, u_r);
}
void GetRMuMuSNuFromScatteringTextureUvwz(in AtmosphereParameters atmosphere, in vec4 uvwz, out float r, 
	out float mu, out float mu_s, out float nu, out bool ray_r_mu_intersects_ground) 
{
	// Distance to top atmosphere boundary for a horizontal ray at ground level.
	float H = sqrt(atmosphere.top_radius * atmosphere.top_radius - atmosphere.bottom_radius * atmosphere.bottom_radius);
	// Distance to the horizon.
	float rho = H * GetUnitRangeFromTextureCoord(uvwz.w, uTexScatteringRSize);
	r = sqrt(rho * rho + atmosphere.bottom_radius * atmosphere.bottom_radius);

	if (uvwz.z < 0.5) 
	{
		// Distance to the ground for the ray (r,mu), and its minimum and maximum
		// values over all mu - obtained for (r,-1) and (r,mu_horizon) - from which
		// we can recover mu:
		float d_min = r - atmosphere.bottom_radius;
		float d_max = rho;
		float d = d_min + (d_max - d_min) * GetUnitRangeFromTextureCoord( 1.0 - 2.0 * uvwz.z, uTexScatteringMuSize / 2);
		mu = d == 0.0 * 1.0 ? float(-1.0) : ClampCosine(-(rho * rho + d * d) / (2.0 * r * d));
		ray_r_mu_intersects_ground = true;
	} 
	else 
	{
		// Distance to the top atmosphere boundary for the ray (r,mu), and its
		// minimum and maximum values over all mu - obtained for (r,1) and
		// (r,mu_horizon) - from which we can recover mu:
		float d_min = atmosphere.top_radius - r;
		float d_max = rho + H;
		float d = d_min + (d_max - d_min) * GetUnitRangeFromTextureCoord( 2.0 * uvwz.z - 1.0, uTexScatteringMuSize / 2);
		mu = d == 0.0 * 1.0 ? float(1.0) : ClampCosine((H * H - rho * rho - d * d) / (2.0 * r * d));
		ray_r_mu_intersects_ground = false;
	}

	float x_mu_s = GetUnitRangeFromTextureCoord(uvwz.y, uTexScatteringMuSSize);
	float d_min = atmosphere.top_radius - atmosphere.bottom_radius;
	float d_max = H;
	float A = -2.0 * atmosphere.mu_s_min * atmosphere.bottom_radius / (d_max - d_min);
	float a = (A - x_mu_s * A) / (1.0 + x_mu_s * A);
	float d = d_min + min(a, A) * (d_max - d_min);
	mu_s = d == 0.0 * 1.0 ? float(1.0) : ClampCosine((H * H - d * d) / (2.0 * atmosphere.bottom_radius * d));

	nu = ClampCosine(uvwz.x * 2.0 - 1.0);
}
void GetRMuMuSNuFromScatteringTextureFragCoord( in AtmosphereParameters atmosphere, in vec3 gl_frag_coord,
	out float r, out float mu, out float mu_s, out float nu, out bool ray_r_mu_intersects_ground) 
{
	vec4 scatteringTexSize = vec4( uTexScatteringNuSize - 1, uTexScatteringMuSSize,
		uTexScatteringMuSize, uTexScatteringRSize);
	float frag_coord_nu =
	floor(gl_frag_coord.x / float(uTexScatteringMuSSize));
	float frag_coord_mu_s = mod(gl_frag_coord.x, float(uTexScatteringMuSSize));
	vec4 uvwz = vec4(frag_coord_nu, frag_coord_mu_s, gl_frag_coord.y, gl_frag_coord.z) / scatteringTexSize;
	GetRMuMuSNuFromScatteringTextureUvwz( atmosphere, uvwz, r, mu, mu_s, nu, ray_r_mu_intersects_ground);
	// Clamp nu to its valid range of values, given mu and mu_s.
	nu = clamp(nu, mu * mu_s - sqrt((1.0 - mu * mu) * (1.0 - mu_s * mu_s)),
	mu * mu_s + sqrt((1.0 - mu * mu) * (1.0 - mu_s * mu_s)));
}

void ComputeSingleScatteringTexture(in AtmosphereParameters atmosphere,
	in sampler2D transmittance_texture, in vec3 gl_frag_coord, out vec3 rayleigh, out vec3 mie) 
{
	float r;
	float mu;
	float mu_s;
	float nu;
	bool ray_r_mu_intersects_ground;
	GetRMuMuSNuFromScatteringTextureFragCoord(atmosphere, gl_frag_coord, r, mu, mu_s, nu, ray_r_mu_intersects_ground);
	ComputeSingleScattering(atmosphere, transmittance_texture, r, mu, mu_s, nu, ray_r_mu_intersects_ground, rayleigh, mie);
}

// single scattering lookup

vec3 GetScattering( in AtmosphereParameters atmosphere, in sampler3D scattering_texture,
    float r, float mu, float mu_s, float nu, bool ray_r_mu_intersects_ground) 
{
	vec4 uvwz = GetScatteringTextureUvwzFromRMuMuSNu( atmosphere, r, mu, mu_s, nu, ray_r_mu_intersects_ground);
	float tex_coord_x = uvwz.x * float(uTexScatteringNuSize - 1);
	float tex_x = floor(tex_coord_x);
	float lerp = tex_coord_x - tex_x;
	vec3 uvw0 = vec3((tex_x + uvwz.y) / float(uTexScatteringNuSize), uvwz.z, uvwz.w);
	vec3 uvw1 = vec3((tex_x + 1.0 + uvwz.y) / float(uTexScatteringNuSize), uvwz.z, uvwz.w);
	return vec3(texture(scattering_texture, uvw0) * (1.0 - lerp) + texture(scattering_texture, uvw1) * lerp);
}
vec3 GetScattering( in AtmosphereParameters atmosphere, in sampler3D single_rayleigh_scattering_texture,
    in sampler3D single_mie_scattering_texture, in sampler3D multiple_scattering_texture, float r, float mu, float mu_s, float nu,
    bool ray_r_mu_intersects_ground, int scattering_order) 
{
	if (scattering_order == 1) 
	{
		vec3 rayleigh = GetScattering( atmosphere, single_rayleigh_scattering_texture, 
			r, mu, mu_s, nu, ray_r_mu_intersects_ground);
		vec3 mie = GetScattering( atmosphere, single_mie_scattering_texture, r, mu, mu_s, nu, ray_r_mu_intersects_ground);
		return rayleigh * RayleighPhaseFunction(nu) + mie * MiePhaseFunction(atmosphere.mie_phase_function_g, nu);
	} 
	else 
	{
		return GetScattering( atmosphere, multiple_scattering_texture, r, mu, mu_s, nu, ray_r_mu_intersects_ground);
	}
}

//***********************
// MULTIPLE SCATTERING
//***********************

// Multiple Scattering Calculation

vec3 GetIrradiance( in AtmosphereParameters atmosphere, in sampler2D irradiance_texture, float r, float mu_s);

vec3 ComputeScatteringDensity( in AtmosphereParameters atmosphere, in sampler2D transmittance_texture,
    in sampler3D single_rayleigh_scattering_texture, in sampler3D single_mie_scattering_texture,
    in sampler3D multiple_scattering_texture, in sampler2D irradiance_texture,
    float r, float mu, float mu_s, float nu, int scattering_order) 
{
  // Compute unit direction vectors for the zenith, the view direction omega and
  // and the sun direction omega_s, such that the cosine of the view-zenith
  // angle is mu, the cosine of the sun-zenith angle is mu_s, and the cosine of
  // the view-sun angle is nu. The goal is to simplify computations below.
  vec3 zenith_direction = vec3(0.0, 0.0, 1.0);
  vec3 omega = vec3(sqrt(1.0 - mu * mu), 0.0, mu);
  float sun_dir_x = omega.x == 0.0 ? 0.0 : (nu - mu * mu_s) / omega.x;
  float sun_dir_y = sqrt(max(1.0 - sun_dir_x * sun_dir_x - mu_s * mu_s, 0.0));
  vec3 omega_s = vec3(sun_dir_x, sun_dir_y, mu_s);

  const int SAMPLE_COUNT = 16;
  const float dphi = PI / float(SAMPLE_COUNT);
  const float dtheta = PI / float(SAMPLE_COUNT);
  vec3 rayleigh_mie = vec3(0.0 * WATT_PER_CUBIC_METER_PER_SR_PER_NM);

  // Nested loops for the integral over all the incident directions omega_i.
  for (int l = 0; l < SAMPLE_COUNT; ++l) 
  {
    float theta = (float(l) + 0.5) * dtheta;
    float cos_theta = cos(theta);
    float sin_theta = sin(theta);
    bool ray_r_theta_intersects_ground = RayIntersectsGround(atmosphere, r, cos_theta);

    // The distance and transmittance to the ground only depend on theta, so we
    // can compute them in the outer loop for efficiency.
    float distance_to_ground = 0.0 * METER;
    vec3 transmittance_to_ground = vec3(0.0);
    vec3 ground_albedo = vec3(0.0);
    if (ray_r_theta_intersects_ground) 
	{
      distance_to_ground = DistanceToBottomAtmosphereBoundary(atmosphere, r, cos_theta);
      transmittance_to_ground = GetTransmittance(atmosphere, transmittance_texture, r, cos_theta, distance_to_ground, true );
      ground_albedo = atmosphere.ground_albedo;
    }

    for (int m = 0; m < 2 * SAMPLE_COUNT; ++m) 
	{
      float phi = (float(m) + 0.5) * dphi;
      vec3 omega_i = vec3(cos(phi) * sin_theta, sin(phi) * sin_theta, cos_theta);
      float domega_i = (dtheta / RAD) * (dphi / RAD) * sin(theta) * STERADIAN;

      // The radiance L_i arriving from direction omega_i after n-1 bounces is
      // the sum of a term given by the precomputed scattering texture for the
      // (n-1)-th order:
      float nu1 = dot(omega_s, omega_i);
      vec3 incident_radiance = GetScattering(atmosphere, single_rayleigh_scattering_texture, single_mie_scattering_texture,
          multiple_scattering_texture, r, omega_i.z, mu_s, nu1, ray_r_theta_intersects_ground, scattering_order - 1);

      // and of the contribution from the light paths with n-1 bounces and whose
      // last bounce is on the ground. This contribution is the product of the
      // transmittance to the ground, the ground albedo, the ground BRDF, and
      // the irradiance received on the ground after n-2 bounces.
      vec3 ground_normal = normalize(zenith_direction * r + omega_i * distance_to_ground);
      vec3 ground_irradiance = GetIrradiance(atmosphere, irradiance_texture, atmosphere.bottom_radius, dot(ground_normal, omega_s));
      incident_radiance += transmittance_to_ground * ground_albedo * (1.0 / (PI * STERADIAN)) * ground_irradiance;

      // The radiance finally scattered from direction omega_i towards direction
      // -omega is the product of the incident radiance, the scattering
      // coefficient, and the phase function for directions omega and omega_i
      // (all this summed over all particle types, i.e. Rayleigh and Mie).
      float nu2 = dot(omega, omega_i);
      float rayleigh_density = GetProfileDensity( atmosphere.rayleigh_density, r - atmosphere.bottom_radius);
      float mie_density = GetProfileDensity( atmosphere.mie_density, r - atmosphere.bottom_radius);
      rayleigh_mie += incident_radiance * ( atmosphere.rayleigh_scattering * rayleigh_density * RayleighPhaseFunction(nu2) +
          atmosphere.mie_scattering * mie_density * MiePhaseFunction(atmosphere.mie_phase_function_g, nu2)) * domega_i;
    }
  }
  return rayleigh_mie;
}

vec3 ComputeMultipleScattering( in AtmosphereParameters atmosphere, in sampler2D transmittance_texture,
    in sampler3D scattering_density_texture, float r, float mu, float mu_s, float nu, bool ray_r_mu_intersects_ground) 
{
	// Number of intervals for the numerical integration.
	const int SAMPLE_COUNT = 50;
	// The integration step, i.e. the length of each integration interval.
	float dx = DistanceToNearestAtmosphereBoundary( atmosphere, r, mu, ray_r_mu_intersects_ground) / float(SAMPLE_COUNT);
	// Integration loop.
	vec3 rayleigh_mie_sum = vec3(0.0 * WATT_PER_SQUARE_METER_PER_SR_PER_NM);
	for (int i = 0; i <= SAMPLE_COUNT; ++i) 
	{
		float d_i = float(i) * dx;

		// The r, mu and mu_s parameters at the current integration point (see the
		// single scattering section for a detailed explanation).
		float r_i = ClampRadius(atmosphere, sqrt(d_i * d_i + 2.0 * r * mu * d_i + r * r));
		float mu_i = ClampCosine((r * mu + d_i) / r_i);
		float mu_s_i = ClampCosine((r * mu_s + d_i * nu) / r_i);

		// The Rayleigh and Mie multiple scattering at the current sample point.
		vec3 rayleigh_mie_i = GetScattering( atmosphere, scattering_density_texture, r_i, mu_i, mu_s_i, nu, ray_r_mu_intersects_ground) 
			* GetTransmittance( atmosphere, transmittance_texture, r, mu, d_i, ray_r_mu_intersects_ground) * dx;
		// Sample weight (from the trapezoidal rule).
		float weight_i = (i == 0 || i == SAMPLE_COUNT) ? 0.5 : 1.0;
		rayleigh_mie_sum += rayleigh_mie_i * weight_i;
	}
	return rayleigh_mie_sum;
}

// Multiple Scattering Precomputation

void GetRMuSFromIrradianceTextureUv(in AtmosphereParameters atmosphere, in vec2 uv, out float r, out float mu_s) 
{
	float x_mu_s = GetUnitRangeFromTextureCoord(uv.x, uTexIrradianceW);
	float x_r = GetUnitRangeFromTextureCoord(uv.y, uTexIrradianceH);
	r = atmosphere.bottom_radius + x_r * (atmosphere.top_radius - atmosphere.bottom_radius);
	mu_s = ClampCosine(2.0 * x_mu_s - 1.0);
}
vec3 ComputeScatteringDensityTexture( in AtmosphereParameters atmosphere, in sampler2D transmittance_texture,
    in sampler3D single_rayleigh_scattering_texture, in sampler3D single_mie_scattering_texture,
    in sampler3D multiple_scattering_texture, in sampler2D irradiance_texture, in vec3 gl_frag_coord, int scattering_order) 
{
	float r;
	float mu;
	float mu_s;
	float nu;
	bool ray_r_mu_intersects_ground;
	GetRMuMuSNuFromScatteringTextureFragCoord(atmosphere, gl_frag_coord, r, mu, mu_s, nu, ray_r_mu_intersects_ground);
	return ComputeScatteringDensity(atmosphere, transmittance_texture, single_rayleigh_scattering_texture, 
		single_mie_scattering_texture, multiple_scattering_texture, irradiance_texture, r, mu, mu_s, nu, scattering_order);
}

vec3 ComputeMultipleScatteringTexture( in AtmosphereParameters atmosphere, in sampler2D transmittance_texture,
    in sampler3D scattering_density_texture, in vec3 gl_frag_coord, out float nu) 
{
  float r;
  float mu;
  float mu_s;
  bool ray_r_mu_intersects_ground;
  GetRMuMuSNuFromScatteringTextureFragCoord(atmosphere, gl_frag_coord, r, mu, mu_s, nu, ray_r_mu_intersects_ground);
  return ComputeMultipleScattering(atmosphere, transmittance_texture,
      scattering_density_texture, r, mu, mu_s, nu, ray_r_mu_intersects_ground);
}

//***********************
// GROUND IRRADIANCE 
//***********************

// ground irradiance computation

vec3 ComputeDirectIrradiance( in AtmosphereParameters atmosphere, in sampler2D transmittance_texture, float r, float mu_s) 
{
	float alpha_s = atmosphere.sun_angular_radius / RAD;
	// Approximate average of the cosine factor mu_s over the visible fraction of
	// the Sun disc.
	float average_cosine_factor = mu_s < -alpha_s ? 0.0 : 
		(mu_s > alpha_s ? mu_s : (mu_s + alpha_s) * (mu_s + alpha_s) / (4.0 * alpha_s));

	return atmosphere.solar_irradiance * GetTransmittanceToTopAtmosphereBoundary(
		atmosphere, transmittance_texture, r, mu_s) * average_cosine_factor;
}

vec3 ComputeIndirectIrradiance( in AtmosphereParameters atmosphere, in sampler3D single_rayleigh_scattering_texture,
    in sampler3D single_mie_scattering_texture, in sampler3D multiple_scattering_texture, float r, float mu_s, int scattering_order) 
{
	const int SAMPLE_COUNT = 32;
	const float dphi = PI / float(SAMPLE_COUNT);
	const float dtheta = PI / float(SAMPLE_COUNT);

	vec3 result = vec3(0.0 * WATT_PER_SQUARE_METER_PER_NM);
	vec3 omega_s = vec3(sqrt(1.0 - mu_s * mu_s), 0.0, mu_s);
	for (int j = 0; j < SAMPLE_COUNT / 2; ++j) 
	{
		float theta = (float(j) + 0.5) * dtheta;
		for (int i = 0; i < 2 * SAMPLE_COUNT; ++i) 
		{
			float phi = (float(i) + 0.5) * dphi;
			vec3 omega = vec3(cos(phi) * sin(theta), sin(phi) * sin(theta), cos(theta));
			float domega = (dtheta / RAD) * (dphi / RAD) * sin(theta) * STERADIAN;

			float nu = dot(omega, omega_s);
			result += GetScattering(atmosphere, single_rayleigh_scattering_texture, single_mie_scattering_texture, 
			multiple_scattering_texture, r, omega.z, mu_s, nu, false, scattering_order) * omega.z * domega;
		}
	}
	return result;
}

// ground irradiance precomputation

vec2 GetIrradianceTextureUvFromRMuS(in AtmosphereParameters atmosphere, float r, float mu_s) 
{
	float x_r = (r - atmosphere.bottom_radius) / (atmosphere.top_radius - atmosphere.bottom_radius);
	float x_mu_s = mu_s * 0.5 + 0.5;
	return vec2(GetTextureCoordFromUnitRange(x_mu_s, uTexIrradianceW), GetTextureCoordFromUnitRange(x_r, uTexIrradianceH));
}

vec2 IRRADIANCE_TEXTURE_SIZE = vec2(uTexIrradianceW, uTexIrradianceH);

vec3 ComputeDirectIrradianceTexture( in AtmosphereParameters atmosphere, in sampler2D transmittance_texture, in vec2 gl_frag_coord) 
{
	float r;
	float mu_s;
	GetRMuSFromIrradianceTextureUv( atmosphere, gl_frag_coord / IRRADIANCE_TEXTURE_SIZE, r, mu_s);
	return ComputeDirectIrradiance(atmosphere, transmittance_texture, r, mu_s);
}
vec3 ComputeIndirectIrradianceTexture( in AtmosphereParameters atmosphere,
    in sampler3D single_rayleigh_scattering_texture, in sampler3D single_mie_scattering_texture,
    in sampler3D multiple_scattering_texture, in vec2 gl_frag_coord, int scattering_order) 
{
	float r;
	float mu_s;
	GetRMuSFromIrradianceTextureUv( atmosphere, gl_frag_coord / IRRADIANCE_TEXTURE_SIZE, r, mu_s);
	return ComputeIndirectIrradiance(atmosphere, single_rayleigh_scattering_texture, single_mie_scattering_texture,
		multiple_scattering_texture, r, mu_s, scattering_order);
}

// ground irradiance lookup

vec3 GetIrradiance( in AtmosphereParameters atmosphere, in sampler2D irradiance_texture, float r, float mu_s) 
{
	vec2 uv = GetIrradianceTextureUvFromRMuS(atmosphere, r, mu_s);
	return vec3(texture(irradiance_texture, uv));
}

//************************
// RENDERING
//************************

//#ifdef COMBINED_SCATTERING_TEXTURES
vec3 GetExtrapolatedSingleMieScattering( in AtmosphereParameters atmosphere, in vec4 scattering) 
{
	if (scattering.r == 0.0) 
	{
		return vec3(0.0);
	}
	return scattering.rgb * scattering.a / scattering.r *
		(atmosphere.rayleigh_scattering.r / atmosphere.mie_scattering.r) *
		(atmosphere.mie_scattering / atmosphere.rayleigh_scattering);
}
//#endif

vec3 GetCombinedScattering( in AtmosphereParameters atmosphere, in sampler3D scattering_texture,
    in sampler3D single_mie_scattering_texture, float r, float mu, float mu_s, float nu, 
	bool ray_r_mu_intersects_ground, out vec3 single_mie_scattering) 
{ 
	vec4 uvwz = GetScatteringTextureUvwzFromRMuMuSNu( atmosphere, r, mu, mu_s, nu, ray_r_mu_intersects_ground);
	float tex_coord_x = uvwz.x * float(uTexScatteringNuSize - 1);
	float tex_x = floor(tex_coord_x);
	float lerp = tex_coord_x - tex_x;
	vec3 uvw0 = vec3((tex_x + uvwz.y) / float(uTexScatteringNuSize), uvwz.z, uvwz.w);
	vec3 uvw1 = vec3((tex_x + 1.0 + uvwz.y) / float(uTexScatteringNuSize), uvwz.z, uvwz.w);
//#ifdef COMBINED_SCATTERING_TEXTURES
	vec4 combined_scattering = texture(scattering_texture, uvw0) * (1.0 - lerp) + texture(scattering_texture, uvw1) * lerp;
	single_mie_scattering = GetExtrapolatedSingleMieScattering(atmosphere, combined_scattering);
//#else
//	vec3 scattering = vec3( texture(scattering_texture, uvw0) * (1.0 - lerp) + 
//		texture(scattering_texture, uvw1) * lerp);
//	single_mie_scattering = vec3( texture(single_mie_scattering_texture, uvw0) * (1.0 - lerp) +
//		texture(single_mie_scattering_texture, uvw1) * lerp);
//#endif
	return combined_scattering.xyz;
}

// SKY

vec3 GetSkyRadiance( in AtmosphereParameters atmosphere, in sampler2D transmittance_texture, in sampler3D scattering_texture,
    in sampler3D single_mie_scattering_texture, vec3 camera, in vec3 view_ray, float shadow_length, in vec3 sun_direction, out vec3 transmittance) 
{
	// Compute the distance to the top atmosphere boundary along the view ray,
	// assuming the viewer is in space (or NaN if the view ray does not intersect
	// the atmosphere).
	float r = length(camera);
	float rmu = dot(camera, view_ray);
	float distance_to_top_atmosphere_boundary = -rmu - sqrt(rmu * rmu - r * r + atmosphere.top_radius * atmosphere.top_radius);
	// If the viewer is in space and the view ray intersects the atmosphere, move
	// the viewer to the top atmosphere boundary (along the view ray):
	if (distance_to_top_atmosphere_boundary > 0.0 * METER) 
	{
		camera = camera + view_ray * distance_to_top_atmosphere_boundary;
		r = atmosphere.top_radius;
		rmu += distance_to_top_atmosphere_boundary;
	} 
	else if (r > atmosphere.top_radius) 
	{
		// If the view ray does not intersect the atmosphere, simply return 0.
		transmittance = vec3(1.0);
		return vec3(0.0 * WATT_PER_SQUARE_METER_PER_SR_PER_NM);
	}
	// Compute the r, mu, mu_s and nu parameters needed for the texture lookups.
	float mu = rmu / r;
	float mu_s = dot(camera, sun_direction) / r;
	float nu = dot(view_ray, sun_direction);
	bool ray_r_mu_intersects_ground = RayIntersectsGround(atmosphere, r, mu);

	transmittance = ray_r_mu_intersects_ground ? vec3(0.0) : GetTransmittanceToTopAtmosphereBoundary( atmosphere, transmittance_texture, r, mu);
	vec3 single_mie_scattering;
	vec3 scattering;
	if (shadow_length == 0.0 * METER) {
		scattering = GetCombinedScattering( atmosphere, scattering_texture, single_mie_scattering_texture,
			r, mu, mu_s, nu, ray_r_mu_intersects_ground, single_mie_scattering);
	} 
	else 
	{
		// Case of light shafts (shadow_length is the total length noted l in our
		// paper): we omit the scattering between the camera and the point at
		// distance l, by implementing Eq. (18) of the paper (shadow_transmittance
		// is the T(x,x_s) term, scattering is the S|x_s=x+lv term).
		float d = shadow_length;
		float r_p = ClampRadius(atmosphere, sqrt(d * d + 2.0 * r * mu * d + r * r));
		float mu_p = (r * mu + d) / r_p;
		float mu_s_p = (r * mu_s + d * nu) / r_p;

		scattering = GetCombinedScattering( atmosphere, scattering_texture, single_mie_scattering_texture,
			r_p, mu_p, mu_s_p, nu, ray_r_mu_intersects_ground, single_mie_scattering);
		vec3 shadow_transmittance = GetTransmittance(atmosphere, transmittance_texture, r, mu, shadow_length, ray_r_mu_intersects_ground);
		scattering = scattering * shadow_transmittance;
		single_mie_scattering = single_mie_scattering * shadow_transmittance;
	}
	return scattering; //scattering * RayleighPhaseFunction(nu) + single_mie_scattering * MiePhaseFunction(atmosphere.mie_phase_function_g, nu);
}

// AERIAL PERSPECTIVE

vec3 GetSkyRadianceToPoint( in AtmosphereParameters atmosphere, in sampler2D transmittance_texture, in sampler3D scattering_texture,
    in sampler3D single_mie_scattering_texture, vec3 camera, in vec3 point, float shadow_length, in vec3 sun_direction, out vec3 transmittance) 
{
	// Compute the distance to the top atmosphere boundary along the view ray,
	// assuming the viewer is in space (or NaN if the view ray does not intersect
	// the atmosphere).
	vec3 view_ray = normalize(point - camera);
	float r = length(camera);
	float rmu = dot(camera, view_ray);
	float distance_to_top_atmosphere_boundary = -rmu - sqrt(rmu * rmu - r * r + atmosphere.top_radius * atmosphere.top_radius);
	// If the viewer is in space and the view ray intersects the atmosphere, move
	// the viewer to the top atmosphere boundary (along the view ray):
	if (distance_to_top_atmosphere_boundary > 0.0 * METER) 
	{
		camera = camera + view_ray * distance_to_top_atmosphere_boundary;
		r = atmosphere.top_radius;
		rmu += distance_to_top_atmosphere_boundary;
	}

	// Compute the r, mu, mu_s and nu parameters for the first texture lookup.
	float mu = rmu / r;
	float mu_s = dot(camera, sun_direction) / r;
	float nu = dot(view_ray, sun_direction);
	float d = length(point - camera);
	bool ray_r_mu_intersects_ground = RayIntersectsGround(atmosphere, r, mu);

	transmittance = GetTransmittance(atmosphere, transmittance_texture, r, mu, d, ray_r_mu_intersects_ground);

	vec3 single_mie_scattering;
	vec3 scattering = GetCombinedScattering( atmosphere, scattering_texture, single_mie_scattering_texture,
		r, mu, mu_s, nu, ray_r_mu_intersects_ground, single_mie_scattering);

	// Compute the r, mu, mu_s and nu parameters for the second texture lookup.
	// If shadow_length is not 0 (case of light shafts), we want to ignore the
	// scattering along the last shadow_length meters of the view ray, which we
	// do by subtracting shadow_length from d (this way scattering_p is equal to
	// the S|x_s=x_0-lv term in Eq. (17) of our paper).
	d = max(d - shadow_length, 0.0 * METER);
	float r_p = ClampRadius(atmosphere, sqrt(d * d + 2.0 * r * mu * d + r * r));
	float mu_p = (r * mu + d) / r_p;
	float mu_s_p = (r * mu_s + d * nu) / r_p;

	vec3 single_mie_scattering_p;
	vec3 scattering_p = GetCombinedScattering( atmosphere, scattering_texture, single_mie_scattering_texture,
		r_p, mu_p, mu_s_p, nu, ray_r_mu_intersects_ground, single_mie_scattering_p);

	// Combine the lookup results to get the scattering between camera and point.
	vec3 shadow_transmittance = transmittance;
	if (shadow_length > 0.0 * METER) 
	{
		// This is the T(x,x_s) term in Eq. (17) of our paper, for light shafts.
		shadow_transmittance = GetTransmittance(atmosphere, transmittance_texture, r, mu, d, ray_r_mu_intersects_ground);
	}
	scattering = scattering - shadow_transmittance * scattering_p;
	single_mie_scattering = single_mie_scattering - shadow_transmittance * single_mie_scattering_p;
#ifdef COMBINED_SCATTERING_TEXTURES
	single_mie_scattering = GetExtrapolatedSingleMieScattering( atmosphere, vec4(scattering, single_mie_scattering.r));
#endif

	// Hack to avoid rendering artifacts when the sun is below the horizon.
	single_mie_scattering = single_mie_scattering * smoothstep(float(0.0), float(0.01), mu_s);

	return scattering * RayleighPhaseFunction(nu) + single_mie_scattering * MiePhaseFunction(atmosphere.mie_phase_function_g, nu);
}

// Ground

vec3 GetSunAndSkyIrradiance( in AtmosphereParameters atmosphere, in sampler2D transmittance_texture, in sampler2D irradiance_texture,
    in vec3 point, in vec3 normal, in vec3 sun_direction, out vec3 sky_irradiance) 
{
	float r = length(point);
	float mu_s = dot(point, sun_direction) / r;

	// Indirect irradiance (approximated if the surface is not horizontal).
	sky_irradiance = GetIrradiance(atmosphere, irradiance_texture, r, mu_s) * (1.0 + dot(normal, point) / r) * 0.5;

	// Direct irradiance.
	return atmosphere.solar_irradiance * GetTransmittanceToSun( atmosphere, transmittance_texture, r, mu_s) * max(dot(normal, sun_direction), 0.0);
}