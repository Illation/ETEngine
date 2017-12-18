<VERTEX>
	#include "AtmoPrecompVert.glsl"
</VERTEX>
<GEOMETRY>
	#include "AtmoPrecompGeo.glsl"
</GEOMETRY>
<FRAGMENT>
	#version 330 core
	
	#include "../Common.glsl"
	#include "../CommonAtmosphere.glsl"

	layout(location = 0) out vec3 delta_rayleigh;
    layout(location = 1) out vec3 delta_mie;
    layout(location = 2) out vec4 scattering;
    layout(location = 3) out vec3 single_mie_scattering;

    uniform mat3 luminance_from_radiance;
    uniform sampler2D transmittance_texture;
    uniform int layer;

	uniform AtmosphereParameters uAtmosphere;

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

    void main() 
	{
		ComputeSingleScatteringTexture( uAtmosphere, transmittance_texture, vec3(gl_FragCoord.xy, layer + 0.5), delta_rayleigh, delta_mie);
		scattering = vec4(luminance_from_radiance * delta_rayleigh.rgb, (luminance_from_radiance * delta_mie).r);
		single_mie_scattering = luminance_from_radiance * delta_mie;
    }
</FRAGMENT>