<VERTEX>
	#version 330 core
	
	in vec3 position;
	
	out vec3 Texcoord;
	
	uniform mat4 model;
	uniform mat4 worldViewProj;
	
	void main()
	{
		vec4 pos = model*vec4(position, 1.0);
		pos = worldViewProj*pos;
		Texcoord = pos.xy;
		gl_Position = pos;
	}
</VERTEX>
<FRAGMENT>
	#version 330 core
	
	in vec2 Texcoord;
	
	layout (location = 0) out vec4 outColor;
	layout (location = 1) out vec4 brightColor;
	
	uniform vec3 color;
	
	const float maxExposure = 5000;
	
	void main()
	{
		vec3 finalCol = clamp(color, 0.0, maxExposure);	
		outColor = vec4(finalCol, 1.0);
		
		float brightness = dot(outColor.rgb, vec3(0.2126, 0.7152, 0.0722));
		if(brightness > 1.0) brightColor = vec4(outColor.rgb, 1.0);
	}
</FRAGMENT>