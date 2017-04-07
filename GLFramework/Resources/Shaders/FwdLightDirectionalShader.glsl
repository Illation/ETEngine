<VERTEX>
	#version 330 core
	layout (location = 0) in vec3 pos;
	layout (location = 1) in vec2 texCoords;

	out vec2 TexCoords;

	void main()
	{
		TexCoords = texCoords;
		gl_Position = vec4(pos, 1.0);
	}
</VERTEX>
<FRAGMENT>
	#version 330 core
	
	in vec2 TexCoords;
	
	//out
	layout (location = 0) out vec4 outColor;
	layout (location = 1) out vec4 brightColor;//probably going to be done differently
	
	const float maxExposure = 5000;
	const float PI = 3.14159265359;
	
	//Gbuffer
	uniform sampler2D texPosAO;                   // | Pos.x   Pos.y   Pos.z | AO .x |
	uniform sampler2D texNormMetSpec;             // | Nor.x   Nor.y | Met.x | Spc.x |
	uniform sampler2D texBaseColRough;            // | BCo.r   BCo.g   BCo.b | Rou.x |
	
	//Light
	uniform vec3 Direction;
	uniform vec3 Color;
	
	uniform vec3 camPos;
	
	//Gbuffer reading
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
	vec3 FresnelSchlick(float cosTheta, vec3 F0)
	{
		return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
	} 
	float DistributionGGX(vec3 N, vec3 H, float roughness)
	{
		float a      = roughness*roughness;
		float a2     = a*a;
		float NdotH  = max(dot(N, H), 0.0);
		float NdotH2 = NdotH*NdotH;
		
		float nom   = a2;
		float denom = (NdotH2 * (a2 - 1.0) + 1.0);
		denom = PI * denom * denom;
		
		return nom / denom;
	}
	float GeometrySchlickGGX(float NdotV, float roughness)
	{
		float r = (roughness + 1.0);
		float k = (r*r) / 8.0;

		float nom   = NdotV;
		float denom = NdotV * (1.0 - k) + k;
		
		return nom / denom;
	}
	float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
	{
		float NdotV = max(dot(N, V), 0.0);
		float NdotL = max(dot(N, L), 0.0);
		float ggx2  = GeometrySchlickGGX(NdotV, roughness);
		float ggx1  = GeometrySchlickGGX(NdotL, roughness);
		
		return ggx1 * ggx2;
	}
	
	//Lighting function
	vec3 DirLighting(vec3 baseCol, float rough, float metal, vec3 F0, vec3 norm, vec3 viewDir)
	{
		vec3 lightDir = Direction;
		vec3 H = normalize(lightDir+viewDir);
		
		vec3 radiance = Color;
		
		vec3 F  = FresnelSchlick(max(dot(H, viewDir), 0.0), F0);	//Fresnel
		float NDF = DistributionGGX(norm, H, rough); 				//Normalized distribution funciton
		float G   = GeometrySmith(norm, viewDir, lightDir, rough);  //Geometry shadowing
		
		//Calculate how much the light contributes
		vec3 kS = F;
		vec3 kD = vec3(1.0) - kS;
		kD *= 1.0 - metal;
		
		//Cook torrance BRDF
		vec3 nominator 	  = NDF * G * F;
		float denominator = 4 * max(dot(norm, viewDir), 0.0) * max(dot(norm, lightDir), 0.0) + 0.001;
		vec3 brdf		  = nominator / denominator;
		
		// add to outgoing radiance Lo
		float NdotL = max(dot(norm, lightDir), 0.0);                
		return (kD * baseCol / PI + brdf) * radiance * NdotL; 
	}	
	
	void main()
	{
		//Extract data from G-Buffer
		vec3 pos = texture(texPosAO, TexCoords).rgb;
		vec3 norm = decodeNormal(texture(texNormMetSpec, TexCoords).rg);
		vec3 baseCol = texture(texBaseColRough, TexCoords).rgb;
		float rough = texture(texBaseColRough, TexCoords).a;
		float metal = texture(texNormMetSpec, TexCoords).b;
		//float ao = texture(texPosAO, TexCoords).a; //maybe use it??
		
		//precalculations	
		vec3 F0 = vec3(0.04);//for dielectric materials use this simplified constant
		F0 		= mix(F0, baseCol, metal);//for metal we should use the albedo value
		//View dir and reflection
		vec3 viewDir = normalize(camPos - pos);
		
		vec3 finalCol = DirLighting(baseCol, rough, metal, F0, norm, viewDir);
		
		//output
		outColor = vec4(clamp(finalCol, 0.0, maxExposure), 1.0);
		float brightness = dot(outColor.rgb, vec3(0.2126, 0.7152, 0.0722));
		if(brightness > 1.0) brightColor = vec4(outColor.rgb, 1.0);
	}
</FRAGMENT>