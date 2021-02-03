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

	layout(location = 0) out vec3 delta_irradiance;
    layout(location = 1) out vec3 irradiance;

    uniform sampler2D uTexTransmittance;

	uniform AtmosphereParameters uAtmosphere;

    void main() 
	{
		delta_irradiance = ComputeDirectIrradianceTexture( uAtmosphere, uTexTransmittance, gl_FragCoord.xy);
		irradiance = vec3(0.0);
    }
</FRAGMENT>