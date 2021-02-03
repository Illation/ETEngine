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

    uniform mat3 luminance_from_radiance;
    uniform sampler3D uTexRayleigh;
    uniform sampler3D uTexDeltaMie;
    uniform sampler3D uTexMultipleScattering;
    uniform int scattering_order;

	uniform AtmosphereParameters uAtmosphere;

    void main() 
	{
		delta_irradiance = ComputeIndirectIrradianceTexture( uAtmosphere, uTexRayleigh, uTexDeltaMie, 
			uTexMultipleScattering, gl_FragCoord.xy, scattering_order );
		irradiance = luminance_from_radiance * delta_irradiance;
    }
</FRAGMENT>