<VERTEX>
	#version 330 core

	in vec3 position;

	out vec4 Texcoord;

	uniform mat4 model;
	uniform mat4 worldViewProj;

	void main()
	{
		vec4 pos = model*vec4(position, 1.0);
		pos = worldViewProj*pos;
		gl_Position = pos;
		Texcoord = pos;//((pos.xy/pos.w)+vec2(1))*0.5f;
	}
</VERTEX>
<FRAGMENT>
	#version 330 core

	#include "Common.glsl"
	#include "CommonDeferred.glsl"
	#include "CommonPBR.glsl"

	in vec4 Texcoord;

	//out
	layout (location = 0) out vec4 outColor;

	GBUFFER_SAMPLER
	uniform mat4 viewProjInv;

	//Light
	uniform vec3 Position;
	uniform vec3 Color = vec3(10);
	uniform float Radius;

	//Lighting function
	vec3 PointLighting(vec3 baseCol, float rough, float metal, vec3 F0, vec3 pos, vec3 norm, vec3 viewDir)
	{
		vec3 lightDir = Position - pos;
		float dist = length(lightDir);
		dist = min(dist, Radius); //Clamp instead of branching

		lightDir = normalize(lightDir);		//L
		vec3 H = normalize(lightDir+viewDir);

		//Calc attenuation with inv square
		float dividend = 1.0 - pow(dist/Radius, 4);
		dividend = clamp(dividend, 0.0, 1.0);
		float attenuation = (dividend*dividend)/((dist*dist)+1);

		//radiance
		vec3 radiance = Color * attenuation;

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
		vec2 tc = (Texcoord.xyz/Texcoord.w).xy;//+vec2(1))*0.5f;
		vec3 viewRay = (viewProjInv * vec4(tc, 1, 1)).xyz;
		tc += vec2(1);
		tc *= 0.5f;
		UNPACK_GBUFFER(tc, viewRay) //maybe use ao??

		//precalculations
		vec3 F0 = vec3(0.04);//for dielectric materials use this simplified constant
		F0 		= mix(F0, baseCol, metal);//for metal we should use the albedo value
		//View dir and reflection
		vec3 viewDir = -normalize(viewRay);

		vec3 finalCol = PointLighting(baseCol, rough, metal, F0, pos, norm, viewDir);

		//output
		outColor = vec4(clamp(finalCol, 0.0, maxExposure), 1.0);
	}
</FRAGMENT>
