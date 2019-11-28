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
	
	void main()
	{
		outputs.magnitude = star.w;
		outputs.coord = vec4(normalize(star.xyz), 1.0);
	}
</VERTEX>
<GEOMETRY>
	#version 400 core
	#include "CommonSharedVars.glsl"
	
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
	uniform float uAspectRatio = 1;

	void CreateVertex(vec3 pos, float mag, vec2 texCoord)
	{
		vec4 pos4 = (staticViewProjection * vec4(pos, 1.0f)).xyzw;
		pos4.z = pos4.w*0.999999;
		gl_Position = pos4; 
		outputs.magnitude = mag;
		outputs.texCoord = texCoord;
		EmitVertex();
	}
	
	void main()
	{
		//Given Data (Vertex Data)
		vec3 pos = inputs[0].coord.xyz;
		
		float radius = uRadius;

		float rady = radius/uAspectRatio;

		// LT----------RT //TringleStrip (LT > RT > LB, LB > RB > RT)
		// |          / |
		// |       /    |
		// |    /       |
		// | /          |
		// LB----------RB
		vec3 LT = vec3(-radius, -rady, 0);
		vec3 RT = vec3(radius, -rady, 0);
		vec3 LB = vec3(-radius, rady, 0);
		vec3 RB = vec3(radius, rady, 0);

		LT = mat3(viewInv)*LT;
		RT = mat3(viewInv)*RT;
		LB = mat3(viewInv)*LB;
		RB = mat3(viewInv)*RB;

		CreateVertex(pos+LT, inputs[0].magnitude, vec2(0)); 
		CreateVertex(pos+RT, inputs[0].magnitude, vec2(1, 0));
		CreateVertex(pos+LB, inputs[0].magnitude, vec2(0, 1)); 
		CreateVertex(pos+RB, inputs[0].magnitude, vec2(1, 1)); 

		EndPrimitive();
	}	
</GEOMETRY>
<FRAGMENT>
	#version 400 core
	#include "Common.glsl"
	
	in GSO 
	{
		float magnitude;
	    vec2 texCoord;
	} inputs;
	uniform sampler2D uTexture;

	//inverse magnitude equation
	//mag - baseMag = -2.512*log10(flux/baseFlux)
	//-2.512*(mag-baseMag) = log10(flux/baseFlux)
	//10^(-2.512*(mag-baseMag)) = flux/baseFlux
	//flux = baseFlux*10^(-2.512*(mag-baseMag))
	uniform float uBaseFlux;//Brightness of a star at base magnitude
	uniform float uBaseMag = 0.03; //0.03 is relative to vega
	//Calculate stars brightness relative to the base magnitude
	float CalculateFlux(float mag)
	{
		//return 10000000;
		//return (1-((mag+1.44)/8))*10000;
		return pow(3, (-2.512f*(mag-uBaseMag)));
	}

	out vec4 outColor;

	
	void main()
	{
		float brightness = clamp(CalculateFlux(inputs.magnitude) * uBaseFlux, 0, maxExposure); 
		outColor = texture( uTexture, inputs.texCoord );
		outColor *= brightness;
		outColor.a = clamp(outColor.a, 0, 1);
	} 
</FRAGMENT>