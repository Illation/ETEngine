<VERTEX>
	#version 400 core
	
	//in
	layout (location = 0) in vec2 vPosition;
	layout (location = 1) in vec4 vColor; 
	layout (location = 2) in vec2 vTexCoord;

	layout (location = 3) in uint vChannel;

	uniform vec2 uTranslation;
	uniform mat4 uTransform;

	uniform mat4 uViewProjection;

	//out
	out VSO
	{
	    vec4 color;
	    vec2 texCoord;
		
		flat uint channel;
	} outputs;
	
	void main()
	{
		outputs.color = vColor;
		outputs.texCoord = vTexCoord;
		outputs.channel = vChannel;

		vec4 pos = uTransform * vec4(vPosition + uTranslation, 0.0, 1.0);
		gl_Position = uViewProjection * pos;
	}
</VERTEX>
<FRAGMENT>
	#version 400 core
	
	// in
	in VSO
	{
	    vec4 color;
	    vec2 texCoord;
		
		flat uint channel;
	} inputs;

	uniform sampler2D uTexture;

	uniform float uThreshold = 0.5;
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

		if (uUseAntiAliasing)
		{
    		// float screenPxDistance = screenPxRange()*(distVal - 0.5);
    		// float opacity = clamp(screenPxDistance + 0.5, 0.0, 1.0);

			float dist = uThreshold - distVal;
			vec2 ddist = vec2(dFdx(dist), dFdy(dist));
			float pixelDist = dist / length(ddist);
			float opacity = clamp(0.5 - pixelDist, 0.0, 1.0);

			outColor = vec4(inputs.color.rgb, inputs.color.a * opacity);
		}
		else
		{
			if (distVal < uThreshold)
			{
				discard;
			}

			outColor = inputs.color;
		}
	} 
</FRAGMENT>