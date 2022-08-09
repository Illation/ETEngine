<VERTEX>
	#version 330 core
	#include "Shaders/CommonSharedVars.glsl"
	layout (location = 0) in vec3 pos;
	layout (location = 1) in vec2 texCoords;

	out vec2 TexCoords;
	out vec3 ViewRay;
	
	void main()
	{
		TexCoords = texCoords;
		ViewRay = (staticViewProjectionInv * vec4(pos.xy, 1, 1)).xyz;
		gl_Position = vec4(pos, 1.0);
	}
</VERTEX>
<FRAGMENT>
	#version 330 core
	
	#include "Shaders/Common.glsl"
	#include "Shaders/CommonDeferred.glsl"
	#include "Shaders/CommonPBR.glsl"
	
	in vec2 TexCoords;
	in vec3 ViewRay;
	
	//out
	layout (location = 0) out vec4 outColor;
	
	uniform mat4 CameraView;
	
	//Light
	uniform vec3 Direction;
	uniform vec3 Color;
	
	//Shadow
	const int NUM_CASCADES = 3;
	struct Cascade
	{
		mat4 LightVP;
		sampler2DShadow ShadowMap;
		float Distance;
	};
	uniform Cascade cascades[NUM_CASCADES];
	
	uniform int PcfSamples = 2;
	uniform float Bias = 0; // 0.001; //depending on if front face culling is used

	// GBuffer
	uniform sampler2D uTexGBufferA;
	uniform sampler2D uTexGBufferB;
	uniform sampler2D uTexGBufferC;
	
	
	float ShadowFactor(vec3 pos, vec3 normal, float dist)
	{
		if(dist >= cascades[NUM_CASCADES-1].Distance) return 1;
		
		float bias = max(Bias * (1.0 - dot(normal, Direction)), Bias*0.01);
		
		float shadow = 0;
		for(int i = 0; i < NUM_CASCADES; ++i)
		{
			float end = cascades[i].Distance;
			if(dist < end)
			{
				float start = (i == 0) ? 0 : cascades[i - 1].Distance;
				if(dist >= start)
				{
					vec4 lightSpace = cascades[i].LightVP * vec4(pos, 1.0);
					vec3 projCoords = lightSpace.xyz / lightSpace.w;
					projCoords = projCoords * 0.5 + 0.5; 
					
					vec3 pcfMult = vec3(1.0 / textureSize(cascades[i].ShadowMap, 0), 0);
					
					for(int x = -PcfSamples; x <= PcfSamples; ++x)
					{
						for(int y = -PcfSamples; y <= PcfSamples; ++y)
						{
							shadow += texture(cascades[i].ShadowMap, projCoords + vec3(x, y, 0)*pcfMult, bias);        
						}    
					}
					
					int samples = PcfSamples*2+1;
					shadow /= samples*samples;
				}
			}
		}
		
		return shadow;
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
		UNPACK_GBUFFER(TexCoords, ViewRay, uTexGBufferA, uTexGBufferB, uTexGBufferC) //maybe use ao??
		
		//precalculations	
		vec3 F0 = vec3(0.04);//for dielectric materials use this simplified constant
		F0 		= mix(F0, baseCol, metal);//for metal we should use the albedo value
		//View dir and reflection
		vec3 viewDir = -normalize(ViewRay);
		
		vec4 depthPos = vec4(pos, 1.0f);
		depthPos = CameraView * depthPos;
		float dist = depthPos.z;
		
		vec3 finalCol = DirLighting(baseCol, rough, metal, F0, norm, viewDir);
		finalCol *= ShadowFactor(pos, norm, dist);
		
		//if(dist < cascades[2].Distance) finalCol *= vec3(1, 0.35f, 0.35f);
		//if(dist < cascades[1].Distance) finalCol *= vec3(0.35f, 1, 0.35f);
		//if(dist < cascades[0].Distance) finalCol *= vec3(0.35f, 0.35f, 1);
		
		//output
		outColor = vec4(clamp(finalCol, 0.0, maxExposure), 1.0);
	}
</FRAGMENT>