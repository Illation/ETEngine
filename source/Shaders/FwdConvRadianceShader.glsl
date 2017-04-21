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
	layout (location = 0) out vec4 outColor;
	in vec3 localPos;

	uniform samplerCube environmentMap;
	uniform float roughness;
	uniform float resolution = 512.0; // resolution of source cubemap (per face)
	
	const uint SAMPLE_COUNT = 4096u;
	const float PI = 3.14159265359f;
	
	float RadicalInverse_VdC(uint bits) 
	{
		bits = (bits << 16u) | (bits >> 16u);
		bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
		bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
		bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
		bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
		return float(bits) * 2.3283064365386963e-10; // / 0x100000000
	}
	// ----------------------------------------------------------------------------
	vec2 Hammersley(uint i, uint N)
	{
		return vec2(float(i)/float(N), RadicalInverse_VdC(i));
	} 
	vec3 ImportanceSampleGGX(vec2 Xi, vec3 N, float roughness)
	{
		float a = roughness*roughness;
		
		float phi = 2.0 * PI * Xi.x;
		float cosTheta = sqrt((1.0 - Xi.y) / (1.0 + (a*a - 1.0) * Xi.y));
		float sinTheta = sqrt(1.0 - cosTheta*cosTheta);
		
		// from spherical coordinates to cartesian coordinates - halfway vector
		vec3 H;
		H.x = cos(phi) * sinTheta;
		H.y = sin(phi) * sinTheta;
		H.z = cosTheta;
		
		//from tangent-space H vector to world-space sample vector
		vec3 up        = abs(N.z) < 0.999 ? vec3(0.0, 0.0, 1.0) : vec3(1.0, 0.0, 0.0);
		vec3 tangent   = normalize(cross(up, N));
		vec3 bitangent = cross(N, tangent);
		
		vec3 sampleVec = tangent * H.x + bitangent * H.y + N * H.z;
		return normalize(sampleVec);
	}  
	float DistributionGGX(vec3 N, vec3 H, float roughness)
	{
		float a = roughness*roughness;
		float a2 = a*a;
		float NdotH = max(dot(N, H), 0.0);
		float NdotH2 = NdotH*NdotH;

		float nom   = a2;
		float denom = (NdotH2 * (a2 - 1.0) + 1.0);
		denom = PI * denom * denom;

		return nom / denom;
	}

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
				// sample from the environment's mip level based on roughness/pdf
				float D   = DistributionGGX(normal, H, roughness);
				float NdotH = max(dot(normal, H), 0.0);
				float HdotV = max(dot(H, V), 0.0);
				float pdf = D * NdotH / (4.0 * HdotV) + 0.0001; 

				float resolution = 512.0; // resolution of source cubemap (per face)
				float saTexel  = 4.0 * PI / (6.0 * resolution * resolution);
				float saSample = 1.0 / (float(SAMPLE_COUNT) * pdf + 0.0001);

				float mipLevel = roughness == 0.0 ? 0.0 : 0.5 * log2(saSample / saTexel); 
				
				radiance += textureLod(environmentMap, L, mipLevel).rgb * NdotL;
				totalWeight += NdotL;
			}
		}
		radiance = radiance / totalWeight;
	  
		outColor = vec4(radiance, 1.0);
	}
</FRAGMENT>