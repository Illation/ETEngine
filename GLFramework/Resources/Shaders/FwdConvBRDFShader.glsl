<VERTEX>
	#version 330 core
	layout (location = 0) in vec3 position;

	out vec3 localPos;

	uniform mat4 projection;
	uniform mat4 view;

	void main()
	{
		localPos = position;  
		gl_Position =  projection * view * vec4(localPos, 1.0);
	} 
</VERTEX>
<FRAGMENT>
	#version 330 core
	
	#include "Common.glsl"
	#include "CommonPBR.glsl"
	
	layout (location = 0) out vec4 outColor;
	in vec3 localPos;

	uniform samplerCube environmentMap;
	uniform float roughness;
	
	const uint SAMPLE_COUNT = 4096u;

	void main()
	{		
		// the sample direction equals the hemisphere's orientation 
		vec3 normal = normalize(localPos);
		vec3 R = normal;
		vec3 V = R;
		
		float totalWeight = 0.0;   
		vec3 radiance = vec3(0.0);     
		for(uint i = 0u; i < SAMPLE_COUNT; ++i)
		{
			vec2 Xi = Hammersley(i, SAMPLE_COUNT);
			vec3 H  = ImportanceSampleGGX(Xi, normal, roughness);
			vec3 L  = normalize(2.0 * dot(V, H) * H - V);

			float NdotL = max(dot(normal, L), 0.0);
			if(NdotL > 0.0)
			{
				radiance += texture(environmentMap, L).rgb * NdotL;
				totalWeight += NdotL;
			}
		}
		radiance = radiance / totalWeight;
	  
		outColor = vec4(radiance, 1.0);
	}
</FRAGMENT>