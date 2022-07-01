<VERTEX>
	#version 330 core
	#include "Shaders/CommonSharedVars.glsl"

	layout (location = 0) in vec3 pos;
	layout (location = 1) in vec2 texCoords;

	uniform mat4 uTransform;

	out vec2 Texcoord;
	
	void main()
	{
		Texcoord = texCoords;
		gl_Position = viewProjection * (uTransform * vec4(pos, 1.0));
	}
</VERTEX>
<FRAGMENT>
	#version 330 core

	in vec2 Texcoord;
	uniform sampler2D uTexture;
	uniform vec4 uColor;

	out vec4 outColor;
	
	void main()
	{
		outColor = texture(uTexture, Texcoord) * uColor;
	}
</FRAGMENT>