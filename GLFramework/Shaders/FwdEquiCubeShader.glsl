<VERTEX>
	#version 330 core
	layout (location = 0) in vec3 position;

	out vec3 localPos;

	uniform mat4 projection;
	uniform mat4 view;

	void main()
	{
		localPos = position;  
		gl_Position =  projection * view * vec4(localPos, 1.0);
	} 
</VERTEX>
<FRAGMENT>
	#version 330 core
	in vec3 localPos;

	uniform sampler2D equirectangularMap;
	
	layout (location = 0) out vec4 outColor;

	const vec2 invAtan = vec2(0.1591f, 0.3183f);
	vec2 SampleSphericalMap(vec3 v)
	{
		vec2 uv = vec2(atan(v.z, v.x), asin(v.y));
		uv *= invAtan;
		uv += 0.5;
		return uv;
	}

	void main()
	{		
		vec2 uv = SampleSphericalMap(normalize(localPos)); // make sure to normalize localPos
		vec3 color = texture(equirectangularMap, uv).rgb;
		
		outColor = vec4(color, 1.0);
	}
</FRAGMENT>