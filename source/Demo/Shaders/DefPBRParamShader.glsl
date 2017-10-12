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

	layout (location = 0) out vec4 texGBufferB;
	layout (location = 1) out vec4 texGBufferC;

	uniform vec3 baseColor;
	uniform float roughness;
	uniform float metalness;

	void main()
	{
		vec3 norm = normalize(Normal);

		texGBufferB = vec4(encodeNormal(norm), metalness, 1);
		texGBufferC = vec4(baseColor, roughness);
	}
</FRAGMENT>
