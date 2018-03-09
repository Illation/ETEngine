<VERTEX>
	#version 330 core
	layout (location = 0) in vec3 pos;
	layout (location = 1) in vec2 texCoords;
	out vec2 Texcoord;

	void main()
	{
		Texcoord = texCoords;
		gl_Position = vec4(pos, 1.0);
	}
</VERTEX>
<FRAGMENT>
	#version 330 core
	in vec2 Texcoord;
	out vec4 outColor;
	
	uniform sampler2D uTex;
	uniform int uChannel;

	void main()
	{
		vec4 color = vec4(0);
		color[uChannel] = texture(uTex, Texcoord).r;
		outColor = color;
	}
</FRAGMENT>