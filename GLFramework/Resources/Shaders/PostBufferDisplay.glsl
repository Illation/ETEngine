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
	
	layout (location = 0) out vec4 outColor;
	layout (location = 1) out vec4 brightColor;
	
	uniform sampler2D texPosition;
	uniform sampler2D texNormal;
	uniform sampler2D texDiffuse;
	uniform sampler2D texSpecular;
	
	struct PointLight 
	{
		vec3 Position;
		vec3 Color;
		
		float Linear;
		float Quadratic;
	};
	struct DirectionalLight
	{
		vec3 Direction;
		vec3 Color;
	};
	const int NR_POINT_LIGHTS = 50;
	uniform PointLight pointLights[NR_POINT_LIGHTS];
	const int NR_DIR_LIGHTS = 8;
	uniform DirectionalLight dirLights[NR_DIR_LIGHTS];
	
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
		brightColor = vec4(0, 0, 0, 1.0);
	}
</FRAGMENT>