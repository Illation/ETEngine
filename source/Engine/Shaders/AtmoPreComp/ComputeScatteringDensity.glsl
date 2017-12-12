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

	layout(location = 0) out vec3 scattering_density;

    uniform sampler2D transmittance_texture;
    uniform sampler3D single_rayleigh_scattering_texture;
    uniform sampler3D single_mie_scattering_texture;
    uniform sampler3D multiple_scattering_texture;
    uniform sampler2D irradiance_texture;
    uniform int scattering_order;
    uniform int layer;
	
    void main() 
	{
		scattering_density = ComputeScatteringDensityTexture(
			ATMOSPHERE, 
			transmittance_texture, 
			single_rayleigh_scattering_texture,
			single_mie_scattering_texture, 
			multiple_scattering_texture,
			irradiance_texture, 
			vec3(gl_FragCoord.xy, layer + 0.5),
			scattering_order
		);
    }
</FRAGMENT>