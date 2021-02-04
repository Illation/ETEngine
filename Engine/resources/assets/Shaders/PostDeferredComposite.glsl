<VERTEX>
	#version 330 core
	#include "Shaders/CommonSharedVars.glsl"
	layout (location = 0) in vec3 pos;
	layout (location = 1) in vec2 texCoords;
	
	out vec2 Texcoord;
	out vec3 ViewRay;
	
	void main()
	{
		Texcoord = texCoords;
		ViewRay = (staticViewProjectionInv * vec4(pos.xy, 1, 1)).xyz;
		gl_Position = vec4(pos, 1.0);
	}
</VERTEX>
<FRAGMENT>
	#version 330 core
	
	#include "Shaders/Common.glsl"
	#include "Shaders/CommonDeferred.glsl"
	#include "Shaders/CommonPBR.glsl"
	
	in vec2 Texcoord;
	in vec3 ViewRay;
	
	layout (location = 0) out vec4 outColor;
	
	//uniform samplerCube texEnvironment;
	uniform samplerCube uTexIrradiance;
	uniform samplerCube uTexRadiance;
	uniform float		uMaxReflectionLod = 4.0;
	uniform sampler2D   uTexBrdfLut;  
	
	void main()
	{
		//Extract data from G-Buffer
		float alpha = 1.0;
		UNPACK_GBUFFER(Texcoord, ViewRay)

		//View dir and reflection
		vec3 viewDir = -normalize(ViewRay);
		vec3 refl = reflect(-viewDir, norm);
		
		//precalculations	
		vec3 F0 = vec3(0.04);//for dielectric materials use this simplified constant
		F0 = mix(F0, baseCol, metal);//for metal we should use the albedo value
		
		vec3 F = FresnelSchlickRoughness(max(dot(norm, viewDir), 0.0), F0, rough);
		
		vec3 kS = F;
		vec3 kD = (1.0 - kS) * (1.0-metal);
		
		vec3 irradiance = texture(uTexIrradiance, norm).rgb;
		vec3 diffuse = irradiance * baseCol;

		vec3 radianceColor = textureLod(uTexRadiance, refl, rough * uMaxReflectionLod).rgb;
		vec2 envBRDF  = texture(uTexBrdfLut, vec2(max(dot(norm, viewDir), 0.0), rough)).rg;
		vec3 specular = radianceColor * (F * envBRDF.x + envBRDF.y);
		
		vec3 ambient = (kD * diffuse + specular) * ao; 
		
		//clean up
		vec3 finalCol = max(ambient, 0.0);		
		
		outColor = vec4(finalCol, alpha);
	}
</FRAGMENT>