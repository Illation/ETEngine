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

    void main() 
	{
		ComputeSingleScatteringTexture( ATMOSPHERE, transmittance_texture, vec3(gl_FragCoord.xy, layer + 0.5), delta_rayleigh, delta_mie);
		scattering = vec4(luminance_from_radiance * delta_rayleigh.rgb, (luminance_from_radiance * delta_mie).r);
		single_mie_scattering = luminance_from_radiance * delta_mie;
    }
</FRAGMENT>