<VERTEX>
	#version 330 core
	
	in vec3 position;
	
	uniform mat4 model;
	uniform mat4 worldViewProj;
	
	void main()
	{
		vec4 pos = model*vec4(position, 1.0);
		pos = worldViewProj*pos;
		gl_Position = pos;
	}
</VERTEX>
<FRAGMENT>
	#disable
</FRAGMENT>