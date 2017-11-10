<VERTEX>
	#version 330 core
	layout (location = 0) in vec3 pos;
	layout (location = 1) in vec2 texCoords;
	out vec2 Texcoord;

	uniform vec2 uSize;
	uniform vec2 uOffset;
	void main()
	{
		vec2 position = (uSize * pos.xy) + uOffset;
		Texcoord = texCoords;
		gl_Position = vec4(position, pos.z, 1.0);
	}
</VERTEX>
<FRAGMENT>
	#version 330 core
	in vec2 Texcoord;
	out vec4 outColor;
	
	uniform sampler2D uTex;
	void main()
	{
		vec3 color = texture(uTex, Texcoord).rgb;
		outColor = vec4(color, 1);
	}
</FRAGMENT>