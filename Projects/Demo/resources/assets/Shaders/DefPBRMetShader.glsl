<VERTEX>
	#version 330 core
	#include "CommonSharedVars.glsl"
	
	in vec3 position;
	in vec3 normal;
	in vec3 tangent;
	in vec2 texcoord;
	
	out vec3 Position;
	out vec3 Normal;
	out vec3 Tangent;
	out vec2 Texcoord;
	
	uniform mat4 model;
	
	void main()
	{
		Texcoord = texcoord;
		
		mat3 normMat = inverse(mat3(model));
		normMat = transpose(normMat);
		Normal = normalize(normMat*normal);
		Tangent = normalize(normMat*tangent);
		
		vec4 pos = model*vec4(position, 1.0);
		Position = vec3(pos.x, pos.y, pos.z);
		gl_Position = viewProjection *pos;
	}
</VERTEX>
<FRAGMENT>
	#version 330 core
	
	#include "CommonDeferred.glsl"
	
	in vec3 Position;
	in vec3 Normal;
	in vec3 Tangent;
	in vec2 Texcoord;
	
	layout (location = 0) out vec4 texGBufferB;    
	layout (location = 1) out vec4 texGBufferC;    
	
	uniform sampler2D texBaseColor;
	uniform sampler2D texRoughness;
	uniform sampler2D texMetalness;
	uniform sampler2D texAO;
	uniform sampler2D texNormal;
	
	vec3 mapNormal()
	{
		vec3 norm = normalize(Normal);
		
		vec3 tang = normalize(Tangent);
		vec3 binorm = normalize(cross(tang, norm));
		mat3 localAxis = mat3(tang, binorm, norm);
		
		vec3 normSample = ((texture(texNormal, Texcoord).rgb)*2)-vec3(1, 1, 1);
		norm = localAxis*normalize(normSample);
		return normalize(norm);
	}
	
	void main()
	{
		vec3 norm = mapNormal();
			
		vec3 baseCol = texture(texBaseColor, Texcoord).rgb;
		float rough = texture(texRoughness, Texcoord).r;
		float metal = texture(texMetalness, Texcoord).r;
		float ao = texture(texAO, Texcoord).r;
		
		texGBufferB = vec4(encodeNormal(norm), metal, ao);
		texGBufferC = vec4(baseCol, rough);
	}
</FRAGMENT>