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
	const float blurSizeH = 1.0 / 300.0;
	const float blurSizeV = 1.0 / 200.0;
	void main()
	{
		
		vec4 top         = texture(texFramebuffer, vec2(Texcoord.x, Texcoord.y + 1.0 / 200.0));
		vec4 bottom      = texture(texFramebuffer, vec2(Texcoord.x, Texcoord.y - 1.0 / 200.0));
		vec4 left        = texture(texFramebuffer, vec2(Texcoord.x - 1.0 / 300.0, Texcoord.y));
		vec4 right       = texture(texFramebuffer, vec2(Texcoord.x + 1.0 / 300.0, Texcoord.y));
		vec4 topLeft     = texture(texFramebuffer, vec2(Texcoord.x - 1.0 / 300.0, Texcoord.y + 1.0 / 200.0));
		vec4 topRight    = texture(texFramebuffer, vec2(Texcoord.x + 1.0 / 300.0, Texcoord.y + 1.0 / 200.0));
		vec4 bottomLeft  = texture(texFramebuffer, vec2(Texcoord.x - 1.0 / 300.0, Texcoord.y - 1.0 / 200.0));
		vec4 bottomRight = texture(texFramebuffer, vec2(Texcoord.x + 1.0 / 300.0, Texcoord.y - 1.0 / 200.0));
		vec4 sx = -topLeft - 2 * left - bottomLeft + topRight   + 2 * right  + bottomRight;
		vec4 sy = -topLeft - 2 * top  - topRight   + bottomLeft + 2 * bottom + bottomRight;
		vec4 sobel = sqrt(sx * sx + sy * sy);
		outColor = vec4(1-sobel.x, 1-sobel.y, 1-sobel.z, 1);
		float avg = 0.2126 * outColor.r + 0.7152 * outColor.g + 0.0722 * outColor.b;
		outColor = vec4(avg, avg, avg, 1.0)*texture(texFramebuffer, Texcoord);
		
		//outColor = texture(texFramebuffer, Texcoord);//*lineColor;
		
		//vec4 sum = vec4(0.0);
		//for (int x = -4; x <= 4; x++)
		//    for (int y = -4; y <= 4; y++)
		//        sum += texture(
		//            texFramebuffer,
		//            vec2(Texcoord.x + x * blurSizeH, Texcoord.y + y * blurSizeV)
		//        ) / 81.0;
		//outColor = sum;
	}
</FRAGMENT>