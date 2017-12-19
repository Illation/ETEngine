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

	layout(location = 0) out vec3 delta_irradiance;
    layout(location = 1) out vec3 irradiance;

    uniform mat3 luminance_from_radiance;
    uniform sampler3D single_rayleigh_scattering_texture;
    uniform sampler3D single_mie_scattering_texture;
    uniform sampler3D multiple_scattering_texture;
    uniform int scattering_order;

	uniform AtmosphereParameters uAtmosphere;

    void main() 
	{
		delta_irradiance = ComputeIndirectIrradianceTexture(
			uAtmosphere, 
			single_rayleigh_scattering_texture,
			single_mie_scattering_texture, 
			multiple_scattering_texture,
			gl_FragCoord.xy, 
			scattering_order
		);
		irradiance = luminance_from_radiance * delta_irradiance;
    }
</FRAGMENT>