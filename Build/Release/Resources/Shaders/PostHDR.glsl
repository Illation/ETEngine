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
	uniform sampler2D texBright;
	
	uniform float exposure;
    uniform float gamma;
	
	const float windowW = 1.0 / 1280.0;
	const float windowH = 1.0 / 720.0;
	
	const float kernel[121] = float[](
		0			,0			,0			,0			,0.000001	,0.000001	,0.000001	,0			,0			,0			,0
		,0			,0			,0.000001	,0.000014	,0.000055	,0.000088	,0.000055	,0.000014	,0.000001	,0			,0
		,0			,0.000001	,0.000036	,0.000362	,0.001445	,0.002289	,0.001445	,0.000362	,0.000036	,0.000001	,0
		,0			,0.000014	,0.000362	,0.003672	,0.014648	,0.023204	,0.014648	,0.003672	,0.000362	,0.000014	,0
		,0.000001	,0.000055	,0.001445	,0.014648	,0.058433	,0.092564	,0.058433	,0.014648	,0.001445	,0.000055	,0.000001
		,0.000001	,0.000088	,0.002289	,0.023204	,0.092564	,0.146632	,0.092564	,0.023204	,0.002289	,0.000088	,0.000001
		,0.000001	,0.000055	,0.001445	,0.014648	,0.058433	,0.092564	,0.058433	,0.014648	,0.001445	,0.000055	,0.000001
		,0			,0.000014	,0.000362	,0.003672	,0.014648	,0.023204	,0.014648	,0.003672	,0.000362	,0.000014	,0
		,0			,0.000001	,0.000036	,0.000362	,0.001445	,0.002289	,0.001445	,0.000362	,0.000036	,0.000001	,0
		,0			,0			,0.000001	,0.000014	,0.000055	,0.000088	,0.000055	,0.000014	,0.000001	,0			,0
		,0			,0			,0			,0			,0.000001	,0.000001	,0.000001	,0			,0			,0			,0);
		
	void main()
	{
		//bloom gaussian blur
		vec4 bloomColor = vec4(0.0);
		//for (int x = -5; x <= 5; x++)
		//{
		//	int xOffset = x+5;
		//    for (int y = -5; y <= 5; y++)
		//	{
		//		float weight = kernel[xOffset+((y+5)*7)];
		//        bloomColor += texture(
		//            texBright,
		//            vec2(Texcoord.x + x * windowW, Texcoord.y + y * windowH)
		//        )*weight;
		//	}
		//}
		vec3 sampledColor = texture(texColor, Texcoord).rgb;
		
		sampledColor+=bloomColor.rgb;
		
		// Reinhard tone mapping
		vec3 mapped = vec3(1.0) - exp(-sampledColor * exposure);
		// Gamma correction 
		mapped = pow(mapped, vec3(1.0 / gamma));
		
		vec3 adjustedColor = clamp(mapped, 0.0, 1.0);
		outColor = vec4(adjustedColor, 1);
	}
</FRAGMENT>