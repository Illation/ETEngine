<VERTEX>
	#version 330 core
	
	in vec3 position;
	in vec3 normal;
	in vec3 tangent;
	in vec2 texcoord;
	
	out vec3 Position;
	out vec3 Normal;
	out vec3 Tangent;
	out vec2 Texcoord;
	
	uniform mat4 model;
	uniform mat4 worldViewProj;
	
	void main()
	{
		Texcoord = texcoord;
		
		mat3 normMat = inverse(mat3(model));
		normMat = transpose(normMat);
		Normal = normalize(normMat*normal);
		Tangent = normalize(normMat*tangent);
		
		vec4 pos = model*vec4(position, 1.0);
		Position = vec3(pos.x, pos.y, pos.z);
		gl_Position = worldViewProj*pos;
	}
</VERTEX>
<FRAGMENT>
	#version 330 core
	
	in vec3 Position;
	in vec3 Normal;
	in vec3 Tangent;
	in vec2 Texcoord;
	
	layout (location = 0) out vec4 outPosition;                  // | Pos.x   Pos.y   Pos.z | AO .x |
	layout (location = 1) out vec4 outNormal;                    // | Nor.x   Nor.y | Met.x | Spc.x |
	layout (location = 2) out vec4 outColor;                     // | BCo.r   BCo.g   BCo.b | Rou.x |
	layout (location = 3) out vec4 outSpecular;
	
	uniform bool useDifTex;
	uniform sampler2D texDiffuse;
	uniform bool useNormTex;
	uniform sampler2D texNormal;
	uniform bool useSpecTex;
	uniform sampler2D texSpecular;
	
	uniform vec3 diffuseColor;
	uniform vec3 specularColor;
	uniform float specularPower;
	
	vec2 encodeNormal(vec3 norm)
	{
		float scale = 1.7777;
		vec2 enc = norm.xy / (norm.z+1);
		enc /= scale;
		return (enc*0.5)+0.5;
	}
	
	void main()
	{
		float alpha = 1.0;
		
		vec3 norm = normalize(Normal);
		if(useNormTex)
		{
			vec3 tang = normalize(Tangent);
			vec3 binorm = normalize(cross(tang, norm));
			mat3 localAxis = mat3(tang, binorm, norm);
			
			vec4 normSample = ((texture(texNormal, Texcoord))*2)-vec4(1, 1, 1, 1);
			norm = localAxis*normalize(vec3(normSample.x, normSample.y, normSample.z));
			norm = normalize(norm);
		}
		vec3 dif = diffuseColor;
		if(useDifTex)
		{
			vec4 difSample = texture(texDiffuse, Texcoord);
			dif = dif * difSample.rgb;
		}
		vec3 spec = specularColor;
		if(useSpecTex)
		{
			vec4 specSample = texture(texSpecular, Texcoord);
			spec = spec * specSample.rgb;
		}
		
		outColor = vec4(dif, alpha);
		outNormal = vec4(encodeNormal(norm), 0, alpha);
		outSpecular = vec4(spec, specularPower);
		outPosition = vec4(Position, alpha);
	}
</FRAGMENT>