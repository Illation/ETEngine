<VERTEX>
	#version 150
	
	in vec3 position;
	in vec3 color;
	in vec2 texcoord;
	
	out vec3 Color;
	out vec2 Texcoord;
	
	uniform mat4 model;
	uniform mat4 worldViewProj;
	
	uniform vec3 overrideColor;
	
	void main()
	{
		Color = overrideColor * color;
		Texcoord = texcoord;
		vec4 pos = model*vec4(position, 1.0);
		gl_Position = worldViewProj*pos;
	}
</VERTEX>
<FRAGMENT>
	#version 150
	
	in vec3 Color;
	in vec2 Texcoord;
	
	out vec4 outColor;
	
	uniform sampler2D texKitten;
	uniform sampler2D texPuppy;
	
	uniform float time;
	
	void main()
	{
		vec4 colKitten = texture(texKitten, Texcoord);
		vec4 colPuppy = texture(texPuppy, Texcoord);
		vec4 texColor = mix(colKitten, colPuppy, 0.0);
		outColor = vec4(Color, 1.0) * texColor;
	}
</FRAGMENT>