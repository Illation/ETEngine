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
	
	uniform samplerCube texEnvironment;
	
	
	uniform vec3 camPos;
	const float maxExposure = 5000;
	
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
	const int NR_POINT_LIGHTS = 50;
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
	
	vec3 DirLighting(DirectionalLight light, vec3 dif, vec3 spec, float specPow, vec3 norm, vec3 viewDir)
	{
		vec3 diffuse = (dif * light.Color) * Lambert(norm, light.Direction);
		vec3 specular = (spec * light.Color) * Blinn(norm, light.Direction, viewDir, specPow);
		return diffuse + specular;
	}
	vec3 PointLighting(PointLight light, vec3 dif, vec3 spec, float specPow, vec3 pos, vec3 norm, vec3 viewDir)
	{
		vec3 lightDir = -normalize(light.Position - pos);
		float dist = length(light.Position - pos);
		
		if(dist<light.Radius)
		{
			float dividend = 1.0 - pow(dist/light.Radius, 4);
			dividend = clamp(dividend, 0.0, 1.0);
			
			float attenuation = (dividend*dividend)/((dist*dist)+1);
		
			vec3 diffuse = (dif * light.Color) * Lambert(norm, lightDir);
			vec3 specular = (spec * light.Color) * Blinn(norm, lightDir, viewDir, specPow);
			
			return (diffuse + specular) * attenuation;
		}
		return vec3(0);
	}
	
	vec3 Fresnel(vec3 norm, vec3 viewDir)
	{
		float fresnel = abs(dot(norm, viewDir));
		fresnel = 1 - clamp(fresnel, 0.0, 1.0);
		fresnel = pow(fresnel,fresnelPow)*fresnelMult;
	
		float fresnelMask = dot(fresnelUp,norm);
		fresnelMask = clamp(fresnelMask, 0.0, 1.0);
		fresnelMask = pow(1 - fresnelMask,fresnelHard);
		fresnel *= fresnelMask;
		
		return fresnel * fresnelCol;
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
		
		//View dir and reflection
		vec3 viewDir = normalize(pos - camPos);
		vec3 refl = reflect(viewDir, norm);
		vec3 flipRef = refl * vec3(1, -1, 1);
		
		//ao and environment mapping
		vec3 env = textureLod(texEnvironment, flipRef, rough*11.0).rgb;
		vec3 finalCol = (env * Fresnel(norm, viewDir))*ao;
		
		//precalculations
		float phongSpec = 1-rough;
		vec3 specular = vec3(phongSpec, phongSpec, phongSpec);
		float specPow = 1.0+(126.0 * phongSpec);
		
		//calculate lighting
		for(int i = 0; i < NR_POINT_LIGHTS; i++)
		{
			finalCol += PointLighting(pointLights[i], baseCol, specular, specPow, pos, norm, viewDir); 
		}
		for(int i = 0; i < NR_DIR_LIGHTS; i++)
		{
			finalCol += DirLighting(dirLights[i], baseCol, specular, specPow, norm, viewDir);
		}
		
		//clean up
		finalCol = max(finalCol, 0.0);		
		
		outColor = vec4(finalCol, alpha);
		
		float brightness = dot(finalCol, vec3(0.2126, 0.7152, 0.0722));
		if(brightness > 1.0) brightColor = vec4(outColor.rgb, 1.0);
	}
</FRAGMENT>