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
	
	in vec4 Texcoord;
	
	//out
	layout (location = 0) out vec4 outColor;
	
	GBUFFER_SAMPLER
	uniform mat4 viewProjInv;
	
	//Light
	uniform vec3 Position;
	uniform float Radius;
	uniform float SurfaceRadius;
	
	uniform vec3 SunDir;
	
	uniform float EPSILON = 0.0001f;
	
	bool intersectSphere(in vec3 viewDir, in vec3 position, in float radius, inout float ffDist, inout float bfDist)
	{
		// vector from ray origin to center of the sphere
		vec3 sphereDir = position-camPos;
		float spDotSp = dot(sphereDir,sphereDir);
		float radiusSq = radius*radius;
		float sDotV = dot(sphereDir,viewDir);
		float innerOffsetSq = radiusSq - (spDotSp - (sDotV * sDotV));
		float innerOffset = sqrt(innerOffsetSq);
		if(spDotSp <= radiusSq)
		{
			// ray origin inside sphere, hit is ensured
			bfDist = sDotV + innerOffset;
			return true;
		}
		else if(sDotV >= 0 && innerOffsetSq >= 0)
		{
			// ray starts outside in front of sphere but hits
			ffDist = sDotV - innerOffset;
			bfDist = sDotV + innerOffset;
			return true;
		}
		return false;
	}	
	
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
		if(intersectSphere(viewDir, Position, Radius, ffDist, bfDist))
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
				
				//just for debugging
				pos = endPos;
			}
		}
		
		float opacity = pathLength/(Radius);
		opacity = pow(opacity, 1);
		opacity = opacity*10;
		
		//output
		outColor = vec4(vec3(0.5, 0.5, 1)*opacity, 1);
	}
</FRAGMENT>