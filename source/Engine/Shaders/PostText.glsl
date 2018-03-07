<VERTEX>
	#version 400 core
	
	//in
	layout (location = 0) in vec3 position;
	layout (location = 1) in vec4 color; 
	layout (location = 2) in vec2 texCoord;
	layout (location = 3) in vec2 charSize;
	layout (location = 4) in float sizeMult;
	layout (location = 5) in uint channel;
	//out
	out VSO
	{
		flat uint channel;
	    vec3 position;
	    vec4 color;
	    vec2 texCoord;
	    vec2 charSize;
		float sizeMult;
	} outputs;
	
	void main()
	{
		outputs.position = position;
		outputs.color = color;
		outputs.texCoord = texCoord;
		outputs.charSize = charSize;
		outputs.sizeMult = sizeMult;
		outputs.channel = channel;
	}
</VERTEX>
<GEOMETRY>
	#version 400 core
	
	layout(points, invocations = 1) in;
	layout(triangle_strip, max_vertices=4) out;

	in VSO
	{
		flat uint channel;
	    vec3 position;
	    vec4 color;
	    vec2 texCoord;
	    vec2 charSize;
		float sizeMult;
	} inputs[];
	out GSO 
	{
		flat uint channel;
		vec4 color;
		vec2 texCoord;
	} outputs;
	
	uniform mat4 transform;
	uniform vec2 texSize;
	
	void main()
	{
		vec2 cS = inputs[0].charSize;
		vec2 cSm = cS * inputs[0].sizeMult;
		
		vec3 pos = inputs[0].position;
		vec2 tc = inputs[0].texCoord;
		
		
		outputs.channel = inputs[0].channel;
		gl_Position = transform * vec4(pos.x, pos.y + cSm.y, pos.z, 1);
		outputs.color = inputs[0].color;
		outputs.texCoord = tc+vec2(0, cS.y)/texSize;
		EmitVertex();
		
		outputs.channel = inputs[0].channel;
		gl_Position = transform * vec4(pos.x + cSm.x, pos.y + cSm.y, pos.z, 1);
		outputs.color = inputs[0].color;
		outputs.texCoord = tc+vec2(cS.x, cS.y)/texSize;
		EmitVertex();
		
		outputs.channel = inputs[0].channel;
		gl_Position = transform * vec4(pos.x, pos.y, pos.z, 1);
		outputs.color = inputs[0].color;
		outputs.texCoord = tc;
		EmitVertex();
		
		outputs.channel = inputs[0].channel;
		gl_Position = transform * vec4(pos.x + cSm.x, pos.y, pos.z, 1);
		outputs.color = inputs[0].color;
		outputs.texCoord = tc+vec2(cS.x, 0)/texSize;
		EmitVertex();
		
		EndPrimitive();
	}	
</GEOMETRY>
<FRAGMENT>
	#version 400 core
	
	in GSO 
	{
		flat uint channel;
		vec4 color;
		vec2 texCoord;
	} inputs;
	uniform sampler2D fontTex;

	uniform float uThreshold = 0.5f;

	out vec4 outColor;
	
	void main()
	{
		float tValue = texture( fontTex, inputs.texCoord )[inputs.channel];
		if( tValue < uThreshold ) 
		{
			discard;
		}
		vec4 textureColor = inputs.color;// * tValue;
		outColor = textureColor;
	} 
</FRAGMENT>