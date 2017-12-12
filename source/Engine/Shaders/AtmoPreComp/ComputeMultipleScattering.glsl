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

	layout(location = 0) out vec3 delta_multiple_scattering;
    layout(location = 1) out vec4 scattering;

    uniform mat3 luminance_from_radiance;
    uniform sampler2D transmittance_texture;
    uniform sampler3D scattering_density_texture;
    uniform int layer;

    void main() 
	{
		float nu;
		delta_multiple_scattering = ComputeMultipleScatteringTexture(
			ATMOSPHERE, 
			transmittance_texture, 
			scattering_density_texture,
			vec3(gl_FragCoord.xy, layer + 0.5), 
			nu
		);
		scattering = vec4( luminance_from_radiance * delta_multiple_scattering.rgb / RayleighPhaseFunction(nu), 0.0 );
	}
</FRAGMENT>