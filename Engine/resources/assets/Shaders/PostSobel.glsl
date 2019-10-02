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
	
	uniform sampler2D inColorTex;
	
	void make_kernel(inout vec4 n[9])
	{
	    n[0] = textureOffset(inColorTex, Texcoord, ivec2(-1, -1));
	    n[1] = textureOffset(inColorTex, Texcoord, ivec2(0, -1));
	    n[2] = textureOffset(inColorTex, Texcoord, ivec2(1, -1));
	    n[3] = textureOffset(inColorTex, Texcoord, ivec2(-1, 0));
	    n[4] = texture2D(inColorTex, Texcoord);
	    n[5] = textureOffset(inColorTex, Texcoord, ivec2(1, 0));
	    n[6] = textureOffset(inColorTex, Texcoord, ivec2(-1, 1));
	    n[7] = textureOffset(inColorTex, Texcoord, ivec2(0, 1));
	    n[8] = textureOffset(inColorTex, Texcoord, ivec2(1, 1));
	}
	
	void main()
	{
		vec4 n[9];
		make_kernel(n);
		
		vec4 edge_w = n[0] + (2.0f * n[3]) + n[6];
		vec4 edge_e = n[2] + (2.0f * n[5]) + n[8];
		vec4 edge_n = n[0] + (2.0f * n[1]) + n[2];
		vec4 edge_s = n[6] + (2.0f * n[7]) + n[8];

		vec4 diff_h = edge_e - edge_w;
		vec4 diff_v = edge_n - edge_s;
		vec4 sobel = sqrt((diff_h * diff_h) + (diff_v * diff_v));

		outColor = vec4(sobel.rgb / 4.f, 1.0f);
	}
</FRAGMENT>