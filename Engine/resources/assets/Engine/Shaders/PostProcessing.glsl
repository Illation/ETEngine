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
	
	uniform sampler2D texColor;
	
	uniform sampler2D texBloom0;
	uniform sampler2D texBloom1;
	uniform sampler2D texBloom2;
	uniform sampler2D texBloom3;
	uniform sampler2D texBloom4;
	uniform sampler2D texBloom5;
	
	uniform float exposure;
    uniform float gamma;
	
	uniform float bloomMult = 1;
	
	void main()
	{
		vec3 color = texture(texColor, Texcoord).rgb;
		
		vec3 bloom = 	texture(texBloom0, Texcoord).rgb;
		bloom += 		texture(texBloom1, Texcoord).rgb;
		bloom += 		texture(texBloom2, Texcoord).rgb;
		bloom += 		texture(texBloom3, Texcoord).rgb;
		bloom += 		texture(texBloom4, Texcoord).rgb;
		bloom += 		texture(texBloom5, Texcoord).rgb;
		bloom *= 		0.1667f;
		
		color += bloom * bloomMult;
		
		// Reinhard tone mapping
		vec3 mapped = vec3(1.0) - exp(-color * exposure);
		// Gamma correction 
		mapped = pow(mapped, vec3(1.0 / gamma));
		
		vec3 adjustedColor = clamp(mapped, 0.0, 1.0);
		outColor = vec4(adjustedColor, 1);
	}
</FRAGMENT>