<VERTEX>
	#version 400 core
	
	// inputs
	//--------
	// per vertex
	layout (location = 0) in vec2 vPosition;
	layout (location = 1) in vec2 vTexCoord;
	layout (location = 2) in uint vChannel;

	// per instance
	layout (location = 3) in vec2  iOffset;
	layout (location = 4) in vec4  iColor;
	layout (location = 5) in float iSdfThreshold;
	layout (location = 6) in float iMinThreshold;
	layout (location = 7) in uint  iIsBlurred;

	// per drawcall
	uniform vec2 uTranslation;
	uniform mat4 uTransform;

	uniform mat4 uViewProjection;

	// out
	out VSO
	{
	    vec2 texCoord;
		flat uint channel;

	    vec4 color;
		float sdfThreshold;
		float minThreshold;
		flat bool isBlurred;
	} outputs;
	
	void main()
	{
		outputs.texCoord = vTexCoord;
		outputs.channel = vChannel;

		outputs.color = iColor;
		outputs.sdfThreshold = iSdfThreshold;
		outputs.minThreshold = iMinThreshold;
		outputs.isBlurred = iIsBlurred > 0 ? true : false;

		vec4 pos = uTransform * vec4(vPosition + uTranslation + iOffset, 0.0, 1.0);
		gl_Position = uViewProjection * pos;
	}
</VERTEX>
<FRAGMENT>
	#version 400 core
	
	// in
	in VSO
	{
	    vec2 texCoord;
		flat uint channel;

	    vec4 color;
		float sdfThreshold;
		float minThreshold;
		flat bool isBlurred;
	} inputs;

	uniform sampler2D uTexture;

	uniform float uSdfSize = 0.0;
	uniform bool uUseAntiAliasing = true;

	// out
	out vec4 outColor;

	// float screenPxRange() 
	// {
	// 	vec2 unitRange = vec2(uSdfSize)/vec2(textureSize(uTexture, 0));
	// 	vec2 screenTexSize = vec2(1.0)/fwidth(inputs.texCoord);
	// 	return max(0.5*dot(unitRange, screenTexSize), 1.0);
	// }
	
	void main()
	{
		float distVal = texture(uTexture, inputs.texCoord)[inputs.channel];

		if (inputs.isBlurred)
		{
			float opacity = max(distVal - inputs.minThreshold, 0.0) / (1.0 - inputs.minThreshold);
			opacity = min(opacity / inputs.sdfThreshold, 1.0);
			outColor = vec4(inputs.color.rgb, inputs.color.a * opacity);
		}
		else
		{
			if (uUseAntiAliasing)
			{
				// float screenPxDistance = screenPxRange()*(distVal - 0.5);
				// float opacity = clamp(screenPxDistance + 0.5, 0.0, 1.0);

				float dist = inputs.sdfThreshold - distVal;
				vec2 ddist = vec2(dFdx(dist), dFdy(dist));
				float pixelDist = dist / length(ddist);
				float opacity = clamp(0.5 - pixelDist, 0.0, 1.0);

				outColor = vec4(inputs.color.rgb, inputs.color.a * opacity);
			}
			else
			{
				if (distVal < inputs.sdfThreshold)
				{
					discard;
				}

				outColor = inputs.color;
			}
		}
	} 
</FRAGMENT>