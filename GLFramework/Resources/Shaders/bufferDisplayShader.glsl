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
		float alpha = 1.0;
		vec3 finalCol = vec3(1, 1, 1);
		
		if(Texcoord.x<0.5)
		{
			if(Texcoord.y>0.5)
			{
				finalCol = texture(texPosition, vec2(Texcoord.x*2, Texcoord.y*2-1)).rgb;//Position
			}
			else
			{
				finalCol = texture(texDiffuse, vec2(Texcoord.x*2, Texcoord.y*2)).rgb;//Diffuse
			}
		}
		else
		{
			if(Texcoord.y>0.5)
			{
				finalCol = texture(texNormal, vec2(Texcoord.x*2-1, Texcoord.y*2-1)).rgb;//Normal
				finalCol += vec3(1, 1, 1);
				finalCol *= 0.5;
			}
			else
			{
				finalCol = texture(texSpecular, vec2(Texcoord.x*2-1, Texcoord.y*2)).rgb;//Specular
			}
		}
		outColor = vec4(finalCol, alpha);
	}
</FRAGMENT>