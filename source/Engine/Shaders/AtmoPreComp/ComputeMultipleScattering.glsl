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
    uniform sampler2D uTexTransmittance;
    uniform sampler3D uTexDeltaScatteringDensity;
    uniform int layer;

	uniform AtmosphereParameters uAtmosphere;

    void main() 
	{
		float nu;
		delta_multiple_scattering = ComputeMultipleScatteringTexture( uAtmosphere, uTexTransmittance, 
			uTexDeltaScatteringDensity, vec3(gl_FragCoord.xy, layer + 0.5), nu );
		scattering = vec4( luminance_from_radiance * delta_multiple_scattering.rgb / RayleighPhaseFunction(nu), 0.0 );
	}
</FRAGMENT>