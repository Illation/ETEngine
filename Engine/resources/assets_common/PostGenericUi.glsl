<VERTEX>
	// This shader works for both basic RmlUI rendering and all of imgui

	#version 400 core
	
	//in
	layout (location = 0) in vec2 vPosition;
	layout (location = 1) in vec4 vColor; 
	layout (location = 2) in vec2 vTexCoord;

	uniform vec2 uTranslation; // not used for imgui
	uniform mat4 uTransform; // not used for imgui

	uniform mat4 uViewProjection;

	//out
	out VSO
	{
	    vec4 color;
	    vec2 texCoord;
	} outputs;
	
	void main()
	{
		outputs.color = vColor;
		outputs.texCoord = vTexCoord;

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
	} inputs;

	uniform sampler2D uTexture;

	// out
	out vec4 outColor;
	
	void main()
	{
		outColor = texture(uTexture, inputs.texCoord) * inputs.color;
	} 
</FRAGMENT>