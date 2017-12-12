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

    uniform sampler2D transmittance_texture;

    void main() 
	{
		delta_irradiance = ComputeDirectIrradianceTexture( ATMOSPHERE, transmittance_texture, gl_FragCoord.xy);
		irradiance = vec3(0.0);
    }
</FRAGMENT>