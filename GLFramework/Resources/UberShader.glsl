<VERTEX>
	#version 150
	
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
		
		vec4 norm = model*vec4(normal, 1.0);
		Normal = normalize(vec3(norm.x, norm.y, norm.z));
		
		vec4 tang = model*vec4(tangent, 1.0);
		Tangent = normalize(vec3(tang.x, tang.y, tang.z));
		
		vec4 pos = model*vec4(position, 1.0);
		Position = vec3(pos.x, pos.y, pos.z);
		gl_Position = worldViewProj*pos;
	}
</VERTEX>
<FRAGMENT>
	#version 150
	
	in vec3 Position;
	in vec3 Normal;
	in vec3 Tangent;
	in vec2 Texcoord;
	
	out vec4 outColor;
	
	uniform vec3 lightDir;
	uniform vec3 camPos;
	
	uniform bool useDifTex;
	uniform sampler2D texDiffuse;
	uniform bool useNormTex;
	uniform sampler2D texNormal;
	
	uniform vec3 ambientColor;
	uniform vec3 diffuseColor;
	uniform vec3 specularColor;
	uniform float specularPower;
	
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
		
		vec3 diffuse = diffuseColor;
		if(useDifTex)
		{
			vec4 difSample = texture(texDiffuse, Texcoord);
			diffuse = diffuse * vec3(difSample.x, difSample.y, difSample.z);
		}
		float difS = dot(norm, -lightDir);//Lambert
		float halfLambert = difS * 0.5 + 0.5; 
		difS = clamp(difS, 0.0, 1.0);
		diffuse = diffuse * difS;
		vec3 ambient = ambientColor*halfLambert;
		
		vec3 specular = specularColor;
		vec3 refl = reflect(normalize(Position-camPos), norm);
		float specS = clamp(dot(refl, -lightDir), 0.0, 1.0);
		specS = clamp(pow(specS, specularPower), 0.0, 1.0);
		specular = specular * specS;
		
		vec3 finalCol = ambient+diffuse+specular;
		finalCol = clamp(finalCol, 0.0, 1.0);		
		
		outColor = vec4(finalCol, alpha);
	}
</FRAGMENT>