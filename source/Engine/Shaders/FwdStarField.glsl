<VERTEX>
	#version 400 core
	
	//in
	layout (location = 0) in vec4 star;
	//out
	out VSO
	{
		float magnitude;
		vec4 coord;
	} outputs;
	
	uniform mat4 viewProj;
	
	void main()
	{
		outputs.magnitude = star.x;
		outputs.coord = (viewProj*vec4(normalize(star.yzw), 1.0));
	}
</VERTEX>
<GEOMETRY>
	#version 400 core
	
	layout(points, invocations = 1) in;
	layout(triangle_strip, max_vertices=4) out;

	in VSO
	{
		float magnitude;
		vec4 coord;
	} inputs[];
	out GSO 
	{
		float magnitude;
	    vec2 texCoord;
	} outputs;
	uniform float uRadius;

	void CreateVertex(vec2 pos, float mag, vec2 texCoord)
	{
		gl_Position = vec4(pos, 1.0f, 1.0f);
		outputs.magnitude = mag;
		outputs.texCoord = texCoord;
		EmitVertex();
	}
	
	void main()
	{
		//Given Data (Vertex Data)
		vec2 position = inputs[0].coord.xy;
		
		// LT----------RT //TringleStrip (LT > RT > LB, LB > RB > RT)
		// |          / |
		// |       /    |
		// |    /       |
		// | /          |
		// LB----------RB
		float radius = uRadius * (1-((inputs[0].magnitude+1.44f)/8));//*1000;

		if(inputs[0].coord.z>0)
		{
			//VERTEX 1 [LT]
			vec2 pos = position+vec2(-radius, -radius);
			vec2 tc = vec2(0);
			CreateVertex(pos, inputs[0].magnitude, tc); 
			//VERTEX 2 [RT]                                             
			pos = position+vec2(radius,-radius);                           
			tc = vec2(1, 0);                                 
			CreateVertex(pos, inputs[0].magnitude, tc);
			//VERTEX 3 [LB]                                             
			pos = position+vec2(-radius,radius);                           
			tc = vec2(0, 1);                                 
			CreateVertex(pos, inputs[0].magnitude, tc); 
			//VERTEX 4 [RB]                                             
			pos =position+vec2(radius, radius);                         
			tc = vec2(1, 1);                                 
			CreateVertex(pos, inputs[0].magnitude, tc); 
		}
		EndPrimitive();
	}	
</GEOMETRY>
<FRAGMENT>
	#version 400 core
	
	in GSO 
	{
		float magnitude;
	    vec2 texCoord;
	} inputs;
	uniform sampler2D uTexture;
	uniform float uBrightnessMult;

	out vec4 outColor;
	
	void main()
	{
		float brightness = (1-((inputs.magnitude+1.44f)/8)) * uBrightnessMult; 
		outColor = texture( uTexture, inputs.texCoord );
		outColor *= brightness;
	} 
</FRAGMENT>