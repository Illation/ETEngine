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
	
	layout (location = 0) out vec4 outPosition;
	layout (location = 1) out vec4 outNormal;
	layout (location = 2) out vec4 outColor;
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
	
	void main()
	{
		float alpha = 1.0;
		
		outNormal.rgb = normalize(Normal);
		if(useNormTex)
		{
			vec3 tang = normalize(Tangent);
			vec3 binorm = normalize(cross(tang, outNormal.rgb));
			mat3 localAxis = mat3(tang, binorm, outNormal.rgb);
			
			vec4 normSample = ((texture(texNormal, Texcoord))*2)-vec4(1, 1, 1, 1);
			outNormal.rgb = localAxis*normalize(vec3(normSample.x, normSample.y, normSample.z));
			outNormal.rgb = normalize(outNormal.rgb);
		}
		outNormal.a = alpha;
		
		outColor.rgb = diffuseColor;
		if(useDifTex)
		{
			vec4 difSample = texture(texDiffuse, Texcoord);
			outColor.rgb = outColor.rgb * vec3(difSample.x, difSample.y, difSample.z);
		}
		outColor.a = alpha;
		
		outSpecular.rgb = specularColor;
		if(useSpecTex)
		{
			vec4 specSample = texture(texSpecular, Texcoord);
			outSpecular.rgb = outSpecular.rgb * vec3(specSample.x, specSample.y, specSample.z);
		}
		outSpecular.a = specularPower;
		
		outPosition = vec4(Position, alpha);
	}
</FRAGMENT>