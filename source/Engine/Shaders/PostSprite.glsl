<VERTEX>
	#version 400 core
	
	//in
	layout (location = 0) in uint textureId;
	layout (location = 1) in vec4 transformData;
	layout (location = 2) in vec4 transformData2; 
	layout (location = 3) in vec4 color;
	//out
	out VSO
	{
		vec4 transformData;
		vec4 transformData2; 
		vec4 color;
	} outputs;
	
	void main()
	{
		outputs.transformData = transformData;
		outputs.transformData2 = transformData2;
		outputs.color = color;
	}
</VERTEX>
<GEOMETRY>
	#version 400 core
	
	layout(points, invocations = 1) in;
	layout(triangle_strip, max_vertices=4) out;

	in VSO
	{
		vec4 transformData;
		vec4 transformData2; 
		vec4 color;
	} inputs[];
	out GSO 
	{
	    vec4 color;
	    vec2 texCoord;
	} outputs;
	
	uniform mat4 uTransform;

	void CreateVertex(vec3 pos, vec4 col, vec2 texCoord, float rotation, vec2 rotCosSin, vec2 offset, vec2 pivotOffset)
	{
		//Transform to origin
		pos.x -=offset.x;
		pos.y -=offset.y;
		pos.x -=pivotOffset.x;
		pos.y -=pivotOffset.y;
		//Rotate
		vec2 posNew;
		posNew.x = pos.x* rotCosSin.x - pos.y* rotCosSin.y;
		posNew.y = pos.y* rotCosSin.x + pos.x* rotCosSin.y;
		//back to origin
		pos.x=posNew.x+offset.x;
		pos.y=posNew.y+offset.y;

		gl_Position = uTransform * vec4(pos, 1.0f);
		outputs.color = col;
		outputs.texCoord = texCoord;
		EmitVertex();
	}
	
	void main()
	{
		//Given Data (Vertex Data)
		vec3 position = vec3(inputs[0].transformData.x,inputs[0].transformData.y,inputs[0].transformData.z); 
		vec2 offset = vec2(inputs[0].transformData.x,inputs[0].transformData.y); 
		float rotation = inputs[0].transformData.w; 
		vec2 pivot = vec2(inputs[0].transformData2.x,inputs[0].transformData2.y); 
		vec2 scale = vec2(inputs[0].transformData2.z,inputs[0].transformData2.w); 
		vec2 texCoord = vec2(0); 
		
		vec2 rotCosSin = vec2(cos(rotation), sin(rotation));

		vec2 pivOf = pivot*scale;//*offset;
		
		// LT----------RT //TringleStrip (LT > RT > LB, LB > RB > RT)
		// |          / |
		// |       /    |
		// |    /       |
		// | /          |
		// LB----------RB

		//VERTEX 1 [LT]
		vec3 pos = position;
		vec2 tc = texCoord;
		CreateVertex(pos, inputs[0].color, tc, rotation, rotCosSin, offset, pivOf); 
		//VERTEX 2 [RT]                                             
		pos = position+vec3(scale.x,0, 0);                           
		tc = texCoord+vec2(1, 0);                                 
		CreateVertex(pos, inputs[0].color, tc, rotation, rotCosSin, offset, pivOf);
		//VERTEX 3 [LB]                                             
		pos = position+vec3(0,scale.y, 0);                           
		tc = texCoord+vec2(0, 1);                                 
		CreateVertex(pos, inputs[0].color, tc, rotation, rotCosSin, offset, pivOf); 
		//VERTEX 4 [RB]                                             
		pos =position+vec3(scale, 0);                         
		tc = texCoord+vec2(1, 1);                                 
		CreateVertex(pos, inputs[0].color, tc, rotation, rotCosSin, offset, pivOf); 
		
		EndPrimitive();
	}	
</GEOMETRY>
<FRAGMENT>
	#version 400 core
	
	in GSO 
	{
	    vec4 color;
	    vec2 texCoord;
	} inputs;
	uniform sampler2D uTexture;

	out vec4 outColor;
	
	void main()
	{
		outColor = texture( uTexture, inputs.texCoord ) * inputs.color;
	} 
</FRAGMENT>