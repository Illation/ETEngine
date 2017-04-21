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
	layout (location = 0) out vec4 brightColor;
	uniform sampler2D texColor;
	uniform float threshold = 1;
	void main()
	{
		vec4 sampledColor = texture(texColor, Texcoord);
		brightColor = sampledColor;
		float brightness = dot(sampledColor.rgb/threshold, vec3(0.2126, 0.7152, 0.0722));
		if(brightness < 1.0f) brightColor = vec4(vec3(0), 1);
	}
</FRAGMENT>