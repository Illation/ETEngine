<VERTEX>
	#include "Shaders/AtmoPreComp/AtmoPrecompVert.glsl"
</VERTEX>
<GEOMETRY>
	#include "Shaders/AtmoPreComp/AtmoPrecompGeo.glsl"
</GEOMETRY>
<FRAGMENT>
	#version 330 core
	
	#include "Shaders/Common.glsl"
	#include "Shaders/CommonAtmosphere.glsl"

	layout(location = 0) out vec3 transmittance;

	uniform AtmosphereParameters uAtmosphere;

	void GetRMuFromTransmittanceTextureUv(in AtmosphereParameters atmosphere, in vec2 uv, out float r, out float mu) 
	{
		//assert(uv.x >= 0.0 && uv.x <= 1.0);
		//assert(uv.y >= 0.0 && uv.y <= 1.0);
		float x_mu = GetUnitRangeFromTextureCoord(uv.x, uTexTransmittanceW);
		float x_r = GetUnitRangeFromTextureCoord(uv.y, uTexTransmittanceH);
		// Distance to top atmosphere boundary for a horizontal ray at ground level.
		float H = sqrt(atmosphere.top_radius * atmosphere.top_radius - atmosphere.bottom_radius * atmosphere.bottom_radius);
		// Distance to the horizon, from which we can compute r:
		float rho = H * x_r;
		r = sqrt(rho * rho + atmosphere.bottom_radius * atmosphere.bottom_radius);
		// Distance to the top atmosphere boundary for the ray (r,mu), and its minimum
		// and maximum values over all mu - obtained for (r,1) and (r,mu_horizon) -
		// from which we can recover mu:
		float d_min = atmosphere.top_radius - r;
		float d_max = rho + H;
		float d = d_min + x_mu * (d_max - d_min);
		mu = d == 0.0 * 1.0 ? float(1.0) : (H * H - rho * rho - d * d) / (2.0 * r * d);
		mu = ClampCosine(mu);
	}
	vec3 ComputeTransmittanceToTopAtmosphereBoundaryTexture( in AtmosphereParameters atmosphere, in vec2 gl_frag_coord) 
	{
		vec2 texTransmittanceSize = vec2(uTexTransmittanceW, uTexTransmittanceH);
		float r;
		float mu;
		GetRMuFromTransmittanceTextureUv( atmosphere, gl_frag_coord / texTransmittanceSize, r, mu);
		return ComputeTransmittanceToTopAtmosphereBoundary(atmosphere, r, mu);
	}

    void main() 
	{
		transmittance = ComputeTransmittanceToTopAtmosphereBoundaryTexture( uAtmosphere, gl_FragCoord.xy);
    }
</FRAGMENT>