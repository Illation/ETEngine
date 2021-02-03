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

	layout(location = 0) out vec3 delta_rayleigh;
    layout(location = 1) out vec3 delta_mie;
    layout(location = 2) out vec4 scattering;
    layout(location = 3) out vec3 single_mie_scattering;

    uniform mat3 luminance_from_radiance;
    uniform sampler2D uTexTransmittance;
    uniform int layer;

	uniform AtmosphereParameters uAtmosphere;

    void main() 
	{
		ComputeSingleScatteringTexture( uAtmosphere, uTexTransmittance, vec3(gl_FragCoord.xy, layer + 0.5), delta_rayleigh, delta_mie);
		scattering = vec4(luminance_from_radiance * delta_rayleigh.rgb, (luminance_from_radiance * delta_mie).r);
		single_mie_scattering = luminance_from_radiance * delta_mie;
    }
</FRAGMENT>