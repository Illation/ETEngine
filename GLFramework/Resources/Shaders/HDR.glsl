<VERTEX>
	#version 150
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
	#version 150
	in vec2 Texcoord;
	out vec4 outColor;
	uniform sampler2D texFramebuffer;
	uniform float exposure;
    uniform float gamma;
	
	void main()
	{
		vec3 sampledColor = texture(texFramebuffer, Texcoord).rgb;
		
		// Reinhard tone mapping
		vec3 mapped = vec3(1.0) - exp(-sampledColor * exposure);
		// Gamma correction 
		mapped = pow(mapped, vec3(1.0 / gamma));
		
		vec3 adjustedColor = clamp(mapped, 0.0, 1.0);
		outColor = vec4(adjustedColor, 1);
	}
</FRAGMENT>