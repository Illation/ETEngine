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
	
	uniform sampler2D texColor;
	uniform sampler2D texBloom;
	
	uniform float exposure;
    uniform float gamma;
	
	uniform float bloomMult = 1;
	
	void main()
	{
		vec3 color = texture(texColor, Texcoord).rgb;
		vec3 bloom = texture(texBloom, Texcoord).rgb;
		
		color += bloom * bloomMult;
		
		// Reinhard tone mapping
		vec3 mapped = vec3(1.0) - exp(-color * exposure);
		// Gamma correction 
		mapped = pow(mapped, vec3(1.0 / gamma));
		
		vec3 adjustedColor = clamp(mapped, 0.0, 1.0);
		outColor = vec4(adjustedColor, 1);
	}
</FRAGMENT>