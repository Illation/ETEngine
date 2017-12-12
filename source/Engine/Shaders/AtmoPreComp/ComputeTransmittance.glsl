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

	layout(location = 0) out vec3 transmittance;

    void main() 
	{
		transmittance = ComputeTransmittanceToTopAtmosphereBoundaryTexture( ATMOSPHERE, gl_FragCoord.xy);
    }
</FRAGMENT>