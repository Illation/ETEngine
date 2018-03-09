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
	
	uniform sampler2D uTex;
	uniform int uChannel;

	uniform vec2 uResolution;
	uniform float uSpread;
	uniform float uHighRes;

	void main()
	{
		bool local = texture(uTex, Texcoord).r > 0.5; //local value

		//get closest opposite
		vec2 startPos = Texcoord - vec2(uSpread) / uResolution;
		vec2 delta = vec2(1 / (uSpread * uHighRes * 2)) / uResolution;
		float closest = uSpread;
		for(int y = 0; y < int(uSpread * uHighRes * 2); ++y)
		{
			for(int x = 0; x < int(uSpread * uHighRes * 2); ++x)
			{
				vec2 samplePos = startPos + vec2(x, y) * delta;
				float dist = distance(Texcoord*uResolution, samplePos*uResolution);
				if(dist < closest)
				{
					bool sampled = texture(uTex, samplePos).r > 0.5;
					if(sampled != local)
					{
						closest = dist;
					}
				}
			}
		}

		//normalize between 0 and 1
		float diff = closest / (uSpread * 2);
		float val = 0.5 + (local ? diff : -diff);

		//apply to output color
		vec4 color = vec4(0);
		color[uChannel] = val;
		outColor = color;
	}
</FRAGMENT>