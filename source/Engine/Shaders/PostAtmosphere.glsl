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
		Texcoord = pos;
	}
</VERTEX>
<FRAGMENT>
	#version 330 core
	
	#include "Common.glsl"
	#include "CommonDeferred.glsl"
	#include "CommonPBR.glsl"
	#include "CommonAtmosphere.glsl"
	
	in vec4 Texcoord;
	
	//out
	layout (location = 0) out vec4 outColor;
	
	GBUFFER_SAMPLER
	uniform mat4 viewProjInv;
	
	uniform vec3 Position;
	
	uniform vec3 SunDir;
	
	uniform float EPSILON = 0.0001f;
	
	void main()
	{
		vec2 tc = (Texcoord.xyz/Texcoord.w).xy;//+vec2(1))*0.5f;
		vec3 viewRay = (viewProjInv * vec4(tc, 1, 1)).xyz;
		tc += vec2(1);
		tc *= 0.5f;
		UNPACK_GBUFFER(tc, viewRay) //maybe use ao??
		
		//View dir and reflection
		vec3 viewDir = normalize(viewRay);
		
		//Calculate atmoSPHERE intersections
		float ffDist = 0;
		float bfDist = 0;
		float pathLength = 0;
		vec3 attenuation = vec3(0);
		if(intersectSphere(viewDir, Position, camPos, Radius, ffDist, bfDist))
		{
			pathLength = min(length(pos-camPos), bfDist);
			if(pathLength >= ffDist)
			{
				//intersection is behind front edge of atmosphere
				vec3 startPos = camPos + viewDir * ffDist;
				float startPosHeight = length(startPos - Position);
				
				// starting position of path is now ensured to be inside atmosphere
				// was either originally there or has been moved to top boundary
				float muStartPos = dot(startPos, viewDir) / startPosHeight;
				float nuStartPos = dot(viewDir, SunDir);
				float musStartPos = dot(startPos, SunDir) / startPosHeight;
				
				vec3 endPos = camPos + viewDir * pathLength;
				float endPosHeight = length(endPos - Position);
				
				pathLength -= ffDist;
				
				if(pathLength < bfDist-ffDist)
				{
					attenuation = analyticTransmittance(startPosHeight, muStartPos, pathLength);
				}
				else
				{
					// retrieve extinction factor for inifinte ray
					attenuation = analyticTransmittance(startPosHeight, muStartPos, pathLength);
				}
				
				//just for debugging
				pos = endPos;
			}
		}
		
		float opacity = pathLength/(Radius);
		opacity = opacity*10;
		
		//output
		outColor = vec4(attenuation*opacity, 1);
	}
</FRAGMENT>