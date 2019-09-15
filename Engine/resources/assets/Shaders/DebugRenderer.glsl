<VERTEX>
	#version 330 core
	layout (location = 0) in vec3 pos;
	layout (location = 1) in vec4 color;
	out vec4 Color;

	uniform mat4 uViewProj;

	void main()
	{
		Color = color;
		gl_Position = uViewProj*vec4(pos, 1.0);
	}
</VERTEX>
<FRAGMENT>
	#version 330 core
	in vec4 Color;
	out vec4 outColor;
	
	void main()
	{
		outColor = Color;
	}
</FRAGMENT>