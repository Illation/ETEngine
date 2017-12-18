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

//bool intersectSphere(in vec3 viewDir, in vec3 position, in vec3 camPos, in float radius, inout float ffDist, inout float bfDist)
//{
//	// vector from ray origin to center of the sphere
//	vec3 sphereDir = position-camPos;
//	float spDotSp = dot(sphereDir,sphereDir);
//	float radiusSq = radius*radius;
//	float sDotV = dot(sphereDir,viewDir);
//	float innerOffsetSq = radiusSq - (spDotSp - (sDotV * sDotV));
//	float innerOffset = sqrt(innerOffsetSq);
//	if(spDotSp <= radiusSq)
//	{
//		// ray origin inside sphere, hit is ensured
//		bfDist = sDotV + innerOffset;
//		return true;
//	}
//	else if(sDotV >= 0 && innerOffsetSq >= 0)
//	{
//		// ray starts outside in front of sphere but hits
//		ffDist = sDotV - innerOffset;
//		bfDist = sDotV + innerOffset;
//		return true;
//	}
//	return false;
//}	

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
	float k = 3.0 / (16.0 * PI * 1.0);
	return k * (1.0 + nu * nu);
}
float MiePhaseFunction(float g, float nu) 
{
	float k = 3.0 / (8.0 * PI * 1.0) * (1.0 - g * g) / (2.0 + g * g);
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

vec3 GetIrradiance( in AtmosphereParameters atmosphere, in sampler2D irradiance_texture, float r, float mu_s);