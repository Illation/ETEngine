<VERTEX>
	#version 330 core
	layout (location = 0) in vec3 pos;
	layout (location = 1) in vec2 texCoords;
	out vec2 Texcoord;
	
	uniform vec2 uResolution;
	uniform float uSpread;
	void main()
	{
		vec2 spread = vec2(uSpread);
		spread.x *= max(uResolution.y / uResolution.x, 1.0);
		spread.y *= max(uResolution.x / uResolution.y, 1.0);
		vec2 adjustment = ((spread*2) + uResolution) / uResolution;
		
		Texcoord = texCoords - vec2(0.5);
		Texcoord *= adjustment;
		Texcoord += vec2(0.5);

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
		int samplePixCount = int(uSpread * uHighRes);

		vec2 adjustment;
		adjustment.x = uResolution.x / uResolution.y;
		adjustment.y = uResolution.y / uResolution.x;

		vec2 delta = vec2(1 / float(samplePixCount)) * min(adjustment, vec2(1.0));
		float ratio = min(uResolution.x, uResolution.y) / max(uResolution.x, uResolution.y);
		adjustment = max(adjustment, vec2(1.0)) * ratio;

		float closest = length(vec2(uSpread));
		for(int y = -samplePixCount; y <= samplePixCount; ++y)
		{
			for(int x = -samplePixCount; x <= samplePixCount; ++x)
			{
				vec2 offset = vec2(x, y) * delta;
				vec2 samplePos = Texcoord + offset;
				float dist = length(offset * adjustment * uResolution);
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
		float diff = closest / (length(vec2(uSpread)) * 2);
		float val = 0.5 + (local ? diff : -diff);

		//apply to output color
		vec4 color = vec4(0);
		color[uChannel] = val;
		outColor = color;
	}
</FRAGMENT>