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
	
	uniform float fresnelPow = 2.5;
	uniform float fresnelMult = 2.0;
	uniform float fresnelHard = 1.0;
	uniform vec3 fresnelCol = vec3(1, 1, 1);
	uniform vec3 fresnelUp = vec3(0, 1, 0);
	
	struct PointLight 
	{
		vec3 Position;
		vec3 Color;
		
		float Radius;
	};
	struct DirectionalLight
	{
		vec3 Direction;
		vec3 Color;
	};
	const int NR_POINT_LIGHTS = 11;
	uniform PointLight pointLights[NR_POINT_LIGHTS];
	const int NR_DIR_LIGHTS = 1;
	uniform DirectionalLight dirLights[NR_DIR_LIGHTS];
	
	vec3 decodeNormal(vec2 enc)
	{
		float scale = 1.7777;
		vec3 nn =
			vec3(enc, 0)*vec3(2*scale,2*scale,0) +
			vec3(-scale,-scale,1);
		float g = 2.0 / dot(nn.rgb,nn.rgb);
		return vec3(g*nn.xy, g-1);
	}
	
	float Lambert(vec3 norm, vec3 lightDir)
	{
		return max(dot(norm, -lightDir), 0);
	}
	float Blinn(vec3 norm, vec3 lightDir, vec3 viewDir, float specPow)
	{
		vec3 halfVec = -normalize(viewDir + lightDir);
		return pow(max(dot(halfVec, norm), 0.0), specPow);
	}
	
	//Deprecated
	vec3 Fresnel(vec3 norm, vec3 viewDir)
	{
		float fresnel = abs(dot(norm, viewDir));//Deprecated
		fresnel = 1 - clamp(fresnel, 0.0, 1.0);//Deprecated
		fresnel = pow(fresnel,fresnelPow)*fresnelMult;//Deprecated
	
		float fresnelMask = dot(fresnelUp,norm);//Deprecated
		fresnelMask = clamp(fresnelMask, 0.0, 1.0);//Deprecated
		fresnelMask = pow(1 - fresnelMask,fresnelHard);//Deprecated
		fresnel *= fresnelMask;//Deprecated
		
		return fresnel * fresnelCol;//Deprecated
	}//Deprecated
	
	//PBR functions
	vec3 FresnelSchlick(float cosTheta, vec3 F0)
	{
		return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
	}
	vec3 FresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness)
	{
		return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(1.0 - cosTheta, 5.0);
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
	
	vec3 DirLighting(DirectionalLight light, vec3 baseCol, float rough, float metal, vec3 F0, vec3 norm, vec3 viewDir)
	{
		vec3 lightDir = light.Direction;
		vec3 H = normalize(lightDir+viewDir);
		
		vec3 radiance = light.Color;
		
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
	vec3 PointLighting(PointLight light, vec3 baseCol, float rough, float metal, vec3 F0, vec3 pos, vec3 norm, vec3 viewDir)
	{
		vec3 lightDir = light.Position - pos;
		float dist = length(lightDir);			
		
		if(dist<light.Radius)
		{
			lightDir = normalize(lightDir);		//L
			vec3 H = normalize(lightDir+viewDir);
			 
			//Calc attenuation with inv square
			float dividend = 1.0 - pow(dist/light.Radius, 4);
			dividend = clamp(dividend, 0.0, 1.0);
			float attenuation = (dividend*dividend)/((dist*dist)+1);
			
			//radiance
			vec3 radiance = light.Color * attenuation;
			
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
		return vec3(0);
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
		
		vec3 finalCol = ambient;
		
		//calculate lighting
		for(int i = 0; i < NR_POINT_LIGHTS; i++)
		{
			finalCol += PointLighting(pointLights[i], baseCol, rough, metal, F0, pos, norm, viewDir); 
		}
		for(int i = 0; i < NR_DIR_LIGHTS; i++)
		{
			finalCol += DirLighting(dirLights[i], baseCol, rough, metal, F0, norm, viewDir);
		}
		
		//clean up
		finalCol = max(finalCol, 0.0);		
		
		outColor = vec4(finalCol, alpha);
		
		float brightness = dot(finalCol, vec3(0.2126, 0.7152, 0.0722));
		if(brightness > 1.0) brightColor = vec4(outColor.rgb, 1.0);
	}
</FRAGMENT>