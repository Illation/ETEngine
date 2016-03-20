<VERTEX>
	#version 330 core
	in vec2 position;
	in vec2 texcoord;
	out vec2 Texcoord;
	void main()
	{
		Texcoord = texcoord;
		gl_Position = vec4(position, 0.0, 1.0);
	}
</VERTEX>
<FRAGMENT>
	#version 330 core
	in vec2 Texcoord;
	out vec4 outColor;
	
	uniform sampler2D texPosition;
	uniform sampler2D texNormal;
	uniform sampler2D texDiffuse;
	uniform sampler2D texSpecular;
	
	uniform vec3 lightDir;
	uniform vec3 camPos;
	const float maxExposure = 5000;
	const float brightness = 5;
		
	uniform vec3 ambientColor;
	
	void main()
	{
		float alpha = texture(texPosition, Texcoord).a;
		vec3 position = texture(texPosition, Texcoord).rgb;
		vec3 norm = texture(texNormal, Texcoord).rgb;
		vec3 diffuse = texture(texDiffuse, Texcoord).rgb;
		vec3 specular = texture(texSpecular, Texcoord).rgb;
		float specPow = texture(texSpecular, Texcoord).a;
		
		
		float difS = dot(norm, -lightDir);//Lambert
		difS = clamp(difS, 0.0, maxExposure);
		diffuse = diffuse * difS;
		
		float halfLambert = difS * 0.5 + 0.5; 
		vec3 ambient = ambientColor*halfLambert;
		
		vec3 refl = reflect(normalize(position-camPos), norm);
		float specS = clamp(dot(refl, -lightDir), 0.0, maxExposure);
		specS = clamp(pow(specS, specPow), 0.0, maxExposure);
		specular = specular * specS;
		
		vec3 finalCol = ambient+diffuse+specular*brightness;
		finalCol = clamp(finalCol, 0.0, maxExposure);		
		
		outColor = vec4(finalCol, alpha);
	}
</FRAGMENT>