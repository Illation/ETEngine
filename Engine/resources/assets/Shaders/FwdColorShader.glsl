<VERTEX>
	#version 330 core
	#include "Shaders/CommonSharedVars.glsl"
	
	in vec3 position;
	
	uniform mat4 model;
	
	void main()
	{
		vec4 pos = model*vec4(position, 1.0);
		gl_Position = viewProjection*pos;
	}
</VERTEX>
<FRAGMENT>
	#version 330 core

	#include "Shaders/CommonDeferred.glsl"	

	layout (location = 0) out vec4 outColor;
	
	uniform vec4 uColor;
	uniform float uOcclusionFactor = 0.5f;

	uniform vec2 uViewSize;

	uniform sampler2D uTexGBufferA;
	
	void main()
	{
		vec2 coord = gl_FragCoord.xy / uViewSize;
		float gbufferDepth = UNPACK_DEPTH(coord, uTexGBufferA);

		float multiplier = 1.f;
		if (gbufferDepth < gl_FragCoord.z)
		{
			multiplier = uOcclusionFactor;
		}

		outColor = vec4(uColor.rgb * multiplier, 1.f);
	}
</FRAGMENT>