<VERTEX>
	#version 330 core
	layout (location = 0) in vec3 pos;
	layout (location = 1) in vec2 texCoords;

	out vec2 TexCoords;

	void main()
	{
		TexCoords = texCoords;
		gl_Position = vec4(pos, 1.0);
	}
</VERTEX>
<FRAGMENT>
	#version 330 core
	
	#include "Common.glsl"
	#include "CommonPBR.glsl"
	
	layout (location = 0) out vec2 outColor;
	in vec2 TexCoords;

	uniform samplerCube environmentMap;
	uniform float roughness;
	
	const uint SAMPLE_COUNT = 1024u;

	vec2 IntegrateBRDF(float roughness, float NdotV)
	{
		vec3 V;
		V.x = sqrt(1.0 - NdotV*NdotV);
		V.y = 0.0;
		V.z = NdotV;

		float A = 0.0;
		float B = 0.0;

		vec3 N = vec3(0.0, 0.0, 1.0);

		for(uint i = 0u; i < SAMPLE_COUNT; ++i)
		{
			vec2 Xi = Hammersley(i, SAMPLE_COUNT);
			vec3 H  = ImportanceSampleGGX(Xi, N, roughness);
			vec3 L  = normalize(2.0 * dot(V, H) * H - V);

			float NdotL = max(L.z, 0.0);
			float NdotH = max(H.z, 0.0);
			float VdotH = max(dot(V, H), 0.0);

			if(NdotL > 0.0)
			{
				float G = GeometrySmithAlt(N, V, L, roughness);
				float G_Vis = (G * VdotH) / (NdotH * NdotV);
				float Fc = pow(1.0 - VdotH, 5.0);

				A += (1.0 - Fc) * G_Vis;
				B += Fc * G_Vis;
			}
		}
		A /= float(SAMPLE_COUNT);
		B /= float(SAMPLE_COUNT);
		return vec2(A, B);
	}
	void main() 
	{
		vec2 radiance = IntegrateBRDF(TexCoords.y, TexCoords.x);
		outColor = radiance;
	}
</FRAGMENT>