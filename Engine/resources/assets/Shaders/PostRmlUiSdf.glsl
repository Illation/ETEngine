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

	// out
	out vec4 outColor;
	
	void main()
	{
		float distVal = texture(uTexture, inputs.texCoord)[inputs.channel];
		if (distVal < uThreshold)
		{
			discard;
		}

		outColor = inputs.color;
	} 
</FRAGMENT>