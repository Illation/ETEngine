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

	layout(location = 0) out vec3 scattering_density;

    uniform sampler2D uTexTransmittance;
    uniform sampler3D uTexRayleigh;
    uniform sampler3D uTexDeltaMie;
    uniform sampler3D uTexMultipleScattering;
    uniform sampler2D uTexDeltaIrradiance;
    uniform int scattering_order;
    uniform int layer;

	uniform AtmosphereParameters uAtmosphere;
	
    void main() 
	{
		scattering_density = ComputeScatteringDensityTexture( uAtmosphere, uTexTransmittance, uTexRayleigh, uTexDeltaMie, 
			uTexMultipleScattering, uTexDeltaIrradiance, vec3(gl_FragCoord.xy, layer + 0.5), scattering_order );
    }
</FRAGMENT>