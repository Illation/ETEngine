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

float intersectAtmosphere(float alt, float mu)
{
	float RL = Radius * 1.001;
    float dout = -alt * mu + sqrt(alt * alt * (mu * mu - 1.0) + RL * RL);
    float delta2 = alt * alt * (mu * mu - 1.0) + SurfaceRadius * SurfaceRadius;
    if (delta2 >= 0.0) 
	{
        float din = -alt * mu - sqrt(delta2);
        if (din >= 0.0) 
		{
            dout = min(dout, din);
        }
    }
    return dout;
}

float densityOverPath(in float scaleHeight, in float alt, in float mu)
{
	// if ray below horizon return max density
	float cosHorizon = -sqrt(1.0f - ((SurfaceRadius*SurfaceRadius)/(alt*alt)));
	if(mu < cosHorizon)
	return 1e9;
	float totalDensity = 0.0f;
	float dx = intersectAtmosphere(alt,mu) / float(TRANSMITTANCE_INTEGRAL_SAMPLES);
	float y_j = exp(-(alt-SurfaceRadius)/scaleHeight);
	for (int i = 1; i<=TRANSMITTANCE_INTEGRAL_SAMPLES; ++i)
	{
		float x_i = float(i)*dx;
		float alt_i = sqrt(alt*alt + x_i*x_i + 2.0f*x_i*alt*mu);
		float y_i = exp(-(alt_i-SurfaceRadius)/scaleHeight);
		totalDensity += (y_j+y_i)/2.0f*dx;
		y_j = y_i;
	}

	return totalDensity;
}

float opticalDepth(float H, float r, float mu, float d) 
{
    float a = sqrt((0.5/H)*r);
    vec2 a01 = a*vec2(mu, mu + d / r);
    vec2 a01s = sign(a01);
    vec2 a01sq = a01*a01;
    float x = a01s.y > a01s.x ? exp(a01sq.x) : 0.0;
    vec2 y = a01s / (2.3193*abs(a01) + sqrt(1.52*a01sq + 4.0)) * vec2(1.0, exp(-d/H*(d/(2.0*r)+mu)));
    return sqrt((6.2831*H)*r) * exp((SurfaceRadius-r)/H) * (x + dot(y, vec2(1.0, -1.0)));
}

vec3 analyticTransmittance(float r, float mu, float d) 
{
    return exp(- betaR * opticalDepth(HR, r, mu, d) - betaMEx * opticalDepth(HM, r, mu, d));
}

//vec4 texture4D(sampler3D table, float r, float mu, float muS, float nu)
//{
//	float H = sqrt(Rt * Rt - Rg * Rg);
//	float rho = sqrt(r * r - Rg * Rg);
//#ifdef INSCATTER_NON_LINEAR
//	float rmu = r * mu;
//	float delta = rmu * rmu - r * r + Rg * Rg;
//	vec4 cst = rmu < 0.0 && delta > 0.0 ? vec4(1.0, 0.0, 0.0, 0.5 - 0.5 / float(RES_MU)) : vec4(-1.0, H * H, H, 0.5 + 0.5 / float(RES_MU));
//	float uR = 0.5 / float(RES_R) + rho / H * (1.0 - 1.0 / float(RES_R));
//	float uMu = cst.w + (rmu * cst.x + sqrt(delta + cst.y)) / (rho + cst.z) * (0.5 - 1.0 / float(RES_MU));
//	// paper formula
//	//float uMuS = 0.5 / float(RES_MU_S) + max((1.0 - exp(-3.0 * muS - 0.6)) / (1.0 - exp(-3.6)), 0.0) * (1.0 - 1.0 / float(RES_MU_S));
//	// better formula
//	float uMuS = 0.5 / float(RES_MU_S) + (atan(max(muS, -0.1975) * tan(1.26 * 1.1)) / 1.1 + (1.0 - 0.26)) * 0.5 * (1.0 - 1.0 / float(RES_MU_S));
//#else
//	float uR = 0.5 / float(RES_R) + rho / H * (1.0 - 1.0 / float(RES_R));
//	float uMu = 0.5 / float(RES_MU) + (mu + 1.0) / 2.0 * (1.0 - 1.0 / float(RES_MU));
//	float uMuS = 0.5 / float(RES_MU_S) + max(muS + 0.2, 0.0) / 1.2 * (1.0 - 1.0 / float(RES_MU_S));
//#endif
//	float lerp = (nu + 1.0) / 2.0 * (float(RES_NU) - 1.0);
//	float uNu = floor(lerp);
//	lerp = lerp - uNu;
//	return texture3D(table, vec3((uNu + uMuS) / float(RES_NU), uMu, uR)) * (1.0 - lerp) +
//		   texture3D(table, vec3((uNu + uMuS + 1.0) / float(RES_NU), uMu, uR)) * lerp;
//}
vec4 texture4D(sampler3D table, float r, float mu, float muS, float nu)
{
   	float H = sqrt(Radius * Radius - SurfaceRadius * SurfaceRadius);
   	float rho = sqrt(r * r - SurfaceRadius * SurfaceRadius);

    float rmu = r * mu;
    float delta = rmu * rmu - r * r + SurfaceRadius * SurfaceRadius;
    vec4 cst = (rmu < 0.0 && delta > 0.0) ? vec4(1.0, 0.0, 0.0, 0.5 - 0.5 / RES_MU) : vec4(-1.0, H * H, H, 0.5 + 0.5 / RES_MU);
    float uR = 0.5 / RES_R + rho / H * (1.0 - 1.0 / RES_R);
    float uMu = cst.w + (rmu * cst.x + sqrt(delta + cst.y)) / (rho + cst.z) * (0.5 - 1.0 / float(RES_MU));
    // paper formula
    //float uMuS = 0.5 / RES_MU_S + max((1.0 - exp(-3.0 * muS - 0.6)) / (1.0 - exp(-3.6)), 0.0) * (1.0 - 1.0 / RES_MU_S);
    // better formula
    float uMuS = 0.5 / RES_MU_S + (atan(max(muS, -0.1975) * tan(1.26 * 1.1)) / 1.1 + (1.0 - 0.26)) * 0.5 * (1.0 - 1.0 / RES_MU_S);

    float lep = (nu + 1.0) / 2.0 * (RES_NU - 1.0);
    float uNu = floor(lep);
    lep = lep - uNu;

    return texture(table, vec3((uNu + uMuS) / RES_NU, uMu, uR)) * (1.0 - lep) 
	+ texture(table, vec3((uNu + uMuS + 1.0) / RES_NU, uMu, uR)) * lep;
}

vec3 getMie(vec4 rayMie) 
{	
	// approximated single Mie scattering (cf. approximate Cm in paragraph "Angular precision")
	// rayMie.rgb=C*, rayMie.w=Cm,r
   	return rayMie.rgb * rayMie.w / max(rayMie.r, 1e-4) * (betaR.r / betaR);
}

float phaseFunctionR(float mu) 
{
	// Rayleigh phase function
    return (3.0 / (16.0 * PI)) * (1.0 + mu * mu);
}

float phaseFunctionM(float mu) 
{
	// Mie phase function
   	 return 1.5 * 1.0 / (4.0 * PI) * (1.0 - mieG*mieG) * pow(1.0 + (mieG*mieG) - 2.0*mieG*mu, -3.0/2.0) * (1.0 + mu * mu) / (2.0 + mieG*mieG);
}

vec3 transmittance(sampler2D texTransmittance, float r, float mu) 
{
	// transmittance(=transparency) of atmosphere for infinite ray (r,mu)
	// (mu=cos(view zenith angle)), intersections with ground ignored
   	float uR, uMu;
    uR = sqrt((r - SurfaceRadius) / (Radius - SurfaceRadius));
    uMu = atan((mu + 0.15) / (1.0 + 0.15) * tan(1.5)) / 1.5;
    
    return texture(texTransmittance, vec2(uMu, uR)).rgb;
}

vec3 irradiance(sampler2D texIrradiance, float r, float muS)
{
	float uR = (r - SurfaceRadius) / (Radius - SurfaceRadius);
	float uMuS = (muS + 0.2) / (1.0 + 0.2);

	return texture(texIrradiance, vec2(uMuS, uR)).rgb;
}