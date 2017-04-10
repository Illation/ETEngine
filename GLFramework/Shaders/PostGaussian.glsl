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
	
	uniform sampler2D image;

	uniform bool horizontal;

	uniform float weight[5] = float[] (0.227027, 0.1945946, 0.1216216, 0.054054, 0.016216);
	
	void main()
	{
		vec2 tex_offset = 1.0 / textureSize(image, 0); // gets size of single texel
		vec3 result = texture(image, Texcoord).rgb * weight[0]; // current fragment's contribution
		if(horizontal)
		{
			for(int i = 1; i < 5; ++i)
			{
				result += texture(image, Texcoord + vec2(tex_offset.x * i, 0.0)).rgb * weight[i];
				result += texture(image, Texcoord - vec2(tex_offset.x * i, 0.0)).rgb * weight[i];
			}
		}
		else
		{
			for(int i = 1; i < 5; ++i)
			{
				result += texture(image, Texcoord + vec2(0.0, tex_offset.y * i)).rgb * weight[i];
				result += texture(image, Texcoord - vec2(0.0, tex_offset.y * i)).rgb * weight[i];
			}
		}
		outColor = vec4(result, 1);
	}
</FRAGMENT>