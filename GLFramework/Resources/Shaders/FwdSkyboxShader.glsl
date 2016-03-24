<VERTEX>
	#version 330 core
	
	layout (location = 0) in vec3 position;
	
	out vec3 TexCoords;
	
	uniform mat4 model;
	uniform mat4 worldViewProj;
	
	void main()
	{
		TexCoords = position;
		
		gl_Position = (worldViewProj*(model*vec4(position, 1.0))).xyww;
	}  
</VERTEX>
<FRAGMENT>
	#version 330 core
	
	in vec3 TexCoords;
	
	layout (location = 0) out vec4 outColor;
	layout (location = 1) out vec4 brightColor;
	
	uniform samplerCube skybox;
	
	void main()
	{    
		outColor = texture(skybox, TexCoords);
	}
</FRAGMENT>