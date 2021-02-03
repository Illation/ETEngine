<VERTEX>
	#version 330 core

	#include "Shaders/CommonSharedVars.glsl"
	
	layout (location = 0) in vec3 position;
	
	uniform mat4 model;
	
	uniform sampler2D texHeight;
	uniform float maxHeight = 10.7f;
	
	void main()
	{
		vec3 Tex3 = normalize(position);
		vec2 uv = vec2(atan( Tex3.z, Tex3.x )/6.28318530718, acos( Tex3.y )/3.14159265359f);
		vec3 pos = position+Tex3*(texture(texHeight, uv).r*maxHeight);
		gl_Position = viewProjection * model * vec4(pos, 1.0f);
	}
</VERTEX>
<FRAGMENT>
	#version 330 core

	out vec4 outColor;
	
	void main()
	{
		outColor = vec4(1.0f, 1.0f, 1.0, 1.0f);
	} 
</FRAGMENT>