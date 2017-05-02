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
	
	uniform float EPSILON = 0.0001f;
	
	bool intersectAtmosphere(in vec3 viewDir, inout float offset, inout float maxPathLength)
	{
		offset = 0.0f;
		maxPathLength = 0.0f;
		// vector from ray origin to center of the sphere
		vec3 l = Position-camPos;
		float l2 = dot(l,l);
		// adjust top atmosphere boundary by small epsilon to prevent artifacts
		float r = Radius;// - EPSILON;
		float r2 = r*r;
		float s = dot(l,viewDir);
		if(l2 <= r2)
		{
			// ray origin inside sphere, hit is ensured
			float m2 = l2 - (s * s);
			float q = sqrt(r2 - m2);
			maxPathLength = s + q;
			return true;
		}
		else if(s >= 0)
		{
			// ray starts outside in front of sphere, hit is possible
			float m2 = l2 - (s * s);
			float q2 = r2 - m2;
			if(q2 >= 0)
			{
				// ray hits atmosphere definitely
				float q = sqrt(q2);
				offset = s - q;
				maxPathLength = (s + q) - offset;
				return true;
			}
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
		float offset;
		float maxPathLength;
		bool hit = intersectAtmosphere(viewDir, offset, maxPathLength);
		
		//calculate the position of the back of the atmosphere
		float bfDistance = offset+maxPathLength;
		float terrainDistance = length(pos-camPos);
		if((hit) && (bfDistance < terrainDistance))
		{
			pos = camPos + viewDir * bfDistance;
		}
		else
		{
			maxPathLength = terrainDistance - offset;
		}
		
		float opacity = maxPathLength/(Radius);
		opacity = pow(opacity, 1);
		opacity = opacity*100;
		
		//output
		outColor = vec4(vec3(0.5, 0.5, 1), hit?opacity:0);
	}
</FRAGMENT>