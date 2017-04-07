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
	in vec2 Texcoord;
	
	layout (location = 0) out vec4 outColor;
	layout (location = 1) out vec4 brightColor;
	
	//Gbuffer
	uniform sampler2D texPosAO;                   // | Pos.x   Pos.y   Pos.z | AO .x |
	uniform sampler2D texNormMetSpec;             // | Nor.x   Nor.y | Met.x | Spc.x |
	uniform sampler2D texBaseColRough;            // | BCo.r   BCo.g   BCo.b | Rou.x |
	
	//uniform samplerCube texEnvironment;
	uniform samplerCube texIrradiance;
	uniform samplerCube texEnvRadiance;
	uniform sampler2D   texBRDFLUT;  
	uniform float MAX_REFLECTION_LOD = 4.0;
	
	uniform vec3 camPos;
	const float maxExposure = 5000;
	const float PI = 3.14159265359;
	
	vec3 decodeNormal(vec2 enc)
	{
		float scale = 1.7777;
		vec3 nn =
			vec3(enc, 0)*vec3(2*scale,2*scale,0) +
			vec3(-scale,-scale,1);
		float g = 2.0 / dot(nn.rgb,nn.rgb);
		return vec3(g*nn.xy, g-1);
	}
	
	//PBR functions
	vec3 FresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness)
	{
		return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(1.0 - cosTheta, 5.0);
	}   
	
	void main()
	{
		//Extract data from G-Buffer
		float alpha = 1.0;
		vec3 pos = texture(texPosAO, Texcoord).rgb;
		vec3 norm = decodeNormal(texture(texNormMetSpec, Texcoord).rg);
		vec3 baseCol = texture(texBaseColRough, Texcoord).rgb;
		float rough = texture(texBaseColRough, Texcoord).a;
		float metal = texture(texNormMetSpec, Texcoord).b;
		float ao = texture(texPosAO, Texcoord).a;
		float spec = texture(texNormMetSpec, Texcoord).a;
		
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
		
		float brightness = dot(finalCol, vec3(0.2126, 0.7152, 0.0722));
		if(brightness > 1.0) brightColor = vec4(outColor.rgb, 1.0);
	}
</FRAGMENT>