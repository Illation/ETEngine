<VERTEX>
	#version 330 core
	
	in vec3 position;
	in vec3 normal;
	
	out vec3 Position;
	out vec3 Normal;
	
	uniform mat4 model;
	uniform mat4 worldViewProj;
	
	void main()
	{
		mat3 normMat = inverse(mat3(model));
		normMat = transpose(normMat);
		Normal = normalize(normMat*normal);
		
		vec4 pos = model*vec4(position, 1.0);
		Position = pos.xyz;
		gl_Position = worldViewProj*pos;
	}
</VERTEX>
<FRAGMENT>
	#version 330 core
	
	#include "CommonDeferred.glsl"
	
	in vec3 Position;
	in vec3 Normal;
	
	layout (location = 0) out vec4 outPosAO;                  	// | Pos.x   Pos.y   Pos.z | AO .x |
	layout (location = 1) out vec4 outNormal;                   // | Nor.x   Nor.y | Met.x | Spc.x |
	layout (location = 2) out vec4 outColor;                    // | BCo.r   BCo.g   BCo.b | Rou.x |
	
	uniform vec3 baseColor;
	uniform float roughness;
	uniform float metalness;
	
	void main()
	{
		float alpha = 1.0;
		
		vec3 norm = normalize(Normal);
		
		outPosAO = vec4(Position, 1.0f);
		outNormal = vec4(encodeNormal(norm), metalness, 0.5f);
		outColor = vec4(baseColor, roughness);
	}
</FRAGMENT>