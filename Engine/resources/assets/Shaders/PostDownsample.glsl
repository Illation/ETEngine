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
	#include "Shaders/Common.glsl"
	in vec2 Texcoord;
	layout (location = 0) out vec4 brightColor;
	uniform sampler2D texColor;
	uniform float threshold = 1;
	void main()
	{
		vec4 sampledColor = texture(texColor, Texcoord);
		brightColor = sampledColor;
		float brightness = dot(sampledColor.rgb/threshold, vec3(0.2126, 0.7152, 0.0722));
		if(brightness < 1.0f) brightColor = vec4(vec3(0), 1);
		brightColor = clamp(brightColor, 0, maxExposure);
	}
</FRAGMENT>