<VERTEX>
	#version 330 core
	in vec2 position;
	in vec2 texcoord;
	out vec2 Texcoord;
	void main()
	{
		Texcoord = texcoord;
		gl_Position = vec4(position, 0.0, 1.0);
	}
</VERTEX>
<FRAGMENT>
	#version 330 core
	
	#include "Common.glsl"
	#include "CommonDeferred.glsl"
	#include "CommonPBR.glsl"
	
	in vec2 Texcoord;
	
	layout (location = 0) out vec4 outColor;
	
	GBUFFER_SAMPLER
	
	//uniform samplerCube texEnvironment;
	uniform samplerCube texIrradiance;
	uniform samplerCube texEnvRadiance;
	uniform sampler2D   texBRDFLUT;  
	uniform float MAX_REFLECTION_LOD = 4.0;
	
	uniform vec3 camPos;
	
	void main()
	{
		//Extract data from G-Buffer
		float alpha = 1.0;
		UNPACK_GBUFFER(Texcoord)
		
		//precalculations	
		vec3 F0 = vec3(0.04);//for dielectric materials use this simplified constant
		F0 		= mix(F0, baseCol, metal);//for metal we should use the albedo value
		
		//View dir and reflection
		vec3 viewDir = normalize(camPos - pos);
		vec3 refl = reflect(viewDir, norm);
		refl.z = -refl.z;

		vec3 radianceColor = textureLod(texEnvRadiance, refl,  rough * MAX_REFLECTION_LOD).rgb;
		
		vec3 F        = FresnelSchlickRoughness(max(dot(norm, viewDir), 0.0), F0, rough);
		
		vec3 kS = F;
		vec3 kD = (1.0 - kS) * (1-metal);
		
		vec3 irradiance = texture(texIrradiance, norm).rgb;
		vec3 diffuse    = irradiance * baseCol;
		
		vec2 envBRDF  = texture(texBRDFLUT, vec2(max(dot(norm, viewDir), 0.0), rough)).rg;
		vec3 specular = radianceColor * (F * envBRDF.x + envBRDF.y);
		
		vec3 ambient    = (kD * diffuse + specular) * ao; 
		
		//clean up
		vec3 finalCol = max(ambient, 0.0);		
		
		outColor = vec4(finalCol, alpha);
	}
</FRAGMENT>