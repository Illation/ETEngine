<VERTEX>
	#version 330 core
	#include "CommonSharedVars.glsl"

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
	//source modified from http://imanolfotia.com/blog/update/2017/03/11/ScreenSpaceReflections.html
	#version 400 core

	#include "Common.glsl"
	#include "CommonDeferred.glsl"
	#include "CommonPBR.glsl"
	
	noperspective in vec2 Texcoord;
	in vec3 ViewRay;
	
	layout (location = 0) out vec4 outColor;
	
	uniform sampler2D uFinalImage; 

	const float rayStep = 0.1;
	const float minRayStep = 0.1;
	const float maxSteps = 30;
	//const float searchDist = 5;
	//const float searchDistInv = 0.2;
	const int numBinarySearchSteps = 5;
	//const float maxDDepth = 1.0;
	//const float maxDDepthInv = 1.0;
	const float reflectionSpecularFalloffExponent = 3.0;

	#define Scale vec3(.8)
	uniform float K = 19.19;

	vec3 BinarySearch(inout vec3 dir, inout vec3 hitCoord, inout float dDepth);
	 
	vec4 RayCast(vec3 dir, inout vec3 hitCoord, out float dDepth);

	vec3 hash(vec3 a);

	void main()
	{
		UNPACK_GBUFFER(Texcoord, ViewRay)
		//if(metal < 0.01)
		//	discard;
			
		//precalculations	
		vec3 F0 = vec3(0.04);//for dielectric materials use this simplified constant
		F0 		= mix(F0, baseCol, metal);//for metal we should use the albedo value
		
		vec3 viewNormal = vec3(vec4(norm, 1) * viewInv);
		
		// Reflection vector
		vec3 viewDir = -normalize(ViewRay);
		vec3 refl = normalize(reflect(-viewDir, norm));
		//vec3 refl = normalize(reflect(normalize(pos), normalize(viewNormal)));

		//vec3 Fresnel = fresnelSchlick(max(dot(normalize(viewNormal), normalize(pos)), 0.0), F0);
		vec3 Fresnel = FresnelSchlick(max(dot(norm, viewDir), 0.0), F0);

		float dDepth;
	 
		//vec3 wp = vec3(vec4(pos, 1.0) * viewInv);
		vec3 wp = pos;
		vec3 hitPos = wp;
		vec3 jitt = mix(vec3(0.0), vec3(hash(wp)-vec3(0.5))*1, 1-rough);//probably use roughness here instead
		vec3 ray = vec3(jitt) + refl * max(minRayStep, -pos.y);
		vec4 coords = RayCast(ray, hitPos, dDepth);
	 
		vec2 dCoords = smoothstep(0.2, 0.6, abs(vec2(0.5, 0.5) - coords.xy));
	 
		float screenEdgefactor = clamp(1.0 - (dCoords.x + dCoords.y), 0.0, 1.0);

		float ReflectionMultiplier = pow(metal, reflectionSpecularFalloffExponent) * screenEdgefactor * -refl.z;
		// Get color
		vec3 SSR = textureLod(uFinalImage, coords.xy, 0).rgb * clamp(ReflectionMultiplier, 0.0, 0.9) * Fresnel;  

		outColor = vec4(SSR, 1);
		outColor = vec4(vec3(coords), 1);

		vec3 simpleSSR = vec3(textureLod(uFinalImage, coords.xy, 0).rgb);
		simpleSSR*=Fresnel*screenEdgefactor*pow(rough, reflectionSpecularFalloffExponent);
		if(depth > 0.999999)simpleSSR*=0;
		vec3 comb = texture(uFinalImage, Texcoord).rgb + simpleSSR;
		outColor = vec4(comb, 1);
	}

	vec3 BinarySearch(inout vec3 dir, inout vec3 hitCoord, inout float dDepth)
	{
		float depth;

		vec4 projectedCoord;
	 
		for(int i = 0; i < numBinarySearchSteps; i++)
		{

			projectedCoord = viewProjection * vec4(hitCoord, 1.0);
			projectedCoord.xy /= projectedCoord.w;
			projectedCoord.xy = projectedCoord.xy * 0.5 + 0.5;
	 
			depth = UNPACK_DEPTH(projectedCoord.xy);

			dDepth = hitCoord.z - depth;
			//dDepth = vec3(viewInv * vec4(hitCoord, 1)).z - depth;

			dir *= 0.5;
			if(dDepth > 0.0) hitCoord += dir;
			else hitCoord -= dir;    
		}

		projectedCoord = vec4(hitCoord, 1.0);
		projectedCoord.xy /= projectedCoord.w;
		projectedCoord.xy = projectedCoord.xy * 0.5 + 0.5;
	 
		return vec3(projectedCoord.xy, depth);
	}

	vec4 RayCast(vec3 dir, inout vec3 hitCoord, out float dDepth)
	{
		dir *= rayStep;
	 
		float depth;
		int steps;
		vec4 projectedCoord;
	 
		for(int i = 0; i < maxSteps; i++)
		{
			hitCoord += dir;
	 
			projectedCoord = viewProjection * vec4(hitCoord, 1.0);
			projectedCoord.xy /= projectedCoord.w;
			projectedCoord.xy = projectedCoord.xy * 0.5 + 0.5;
	 
			depth = UNPACK_DEPTH(projectedCoord.xy);
			if(depth > 1000.0)
				continue;
	 
			//dDepth = vec3(viewInv * vec4(hitCoord, 1)).z - depth;
			dDepth = depth;//hitCoord.z - depth;

			if((vec3(viewProjection * vec4(dir, 1)).z - dDepth) < 1.2)
			if(dDepth <= 0.0)
			{   
				return vec4(BinarySearch(dir, hitCoord, dDepth), 1.0);
			}
			
			steps++;
		}
		
		return vec4(projectedCoord.xy, depth, 0.0);
	}

	vec3 hash(vec3 a)
	{
		a = fract(a * Scale);
		a += dot(a, a.yxz + K);
		return fract((a.xxy + a.yxx)*a.zyx);
	}
</FRAGMENT>