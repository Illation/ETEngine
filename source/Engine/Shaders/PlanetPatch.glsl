<VERTEX>
	#version 330 core
	//Patch
	layout (location = 0) in vec2 pos;
	layout (location = 1) in vec2 morph;
	//Instance
	layout (location = 2) in int level;
	layout (location = 3) in vec3 a;
	layout (location = 4) in vec3 r;
	layout (location = 5) in vec3 s;
	//Morph calculation
	uniform vec3 camPos;
	uniform float radius;
	uniform float morphRange;
	uniform float distanceLUT[32];
	//Transformation
	uniform mat4 model;
	uniform mat4 viewProj;
	//Height sampling
	uniform sampler2D texHeight;
	uniform sampler2D texHeightDetail;
	uniform sampler2D texDetail2;
	uniform float maxHeight = 10.7f;
	//outputs
	out vec3 Tex3;
	out vec3 Normal;
	out vec3 TriPos;
	
	float height(vec3 Tex3)
	{
		//return 0;
		vec2 uv = vec2(atan( Tex3.z, Tex3.x )/6.28318530718, acos( Tex3.y )/3.14159265359f);
		vec2 tileStretch = vec2(2, 1);
		float detail1 = (texture(texHeightDetail, uv*tileStretch*100).r)*maxHeight*0.1f;
		float detail2 = (1 - texture(texDetail2, uv*tileStretch*700).r)*0.01f;
		return texture(texHeight, uv).r*maxHeight+detail1+detail2;
	}
	float morphFac(float dist, int lev)
	{
		float low = distanceLUT[lev-1];
		float high = distanceLUT[lev];
		
		float delta = high-low;
		float a = (dist-low)/delta;
		
		return 1 - clamp(a/morphRange, 0, 1);
	}
	void main()
	{
		//initial position
		vec3 TriPos = a + r*pos.x + s*pos.y;
		//morph factor
		float dist = length(TriPos-camPos);
		float mPerc = morphFac(dist, level);
		//morph
		TriPos += mPerc*(r*morph.x + s*morph.y);
		//add height
		Tex3 = normalize(TriPos);
		TriPos = Tex3 * (radius + height(Tex3));
		
		Normal = normalize((model*vec4(Tex3, 1.0f)).xyz);
		gl_Position = viewProj * model * vec4(TriPos, 1.0f);
	}
</VERTEX>
<FRAGMENT>
	#version 330 core
	
	in vec3 Tex3;
	in vec3 Normal;
	in vec3 TriPos;
	
	uniform vec3 camPos;
	
	uniform sampler2D texDiffuse;
	uniform sampler2D texHeight;
	uniform sampler2D texDetail1;
	uniform sampler2D texDetail2;
	uniform sampler2D texHeightDetail;
	
	uniform vec3 lightDir = vec3(-1, -0.3, 1);
	
	uniform vec3 diffuse = vec3(1.0f, 0.5f, 0.2f);
	uniform vec3 ambient = vec3(0.05f, 0.05f, 0.08f);
	
	uniform float maxHeight = 10.7f;
	uniform vec3 texOffset = vec3(1.0f/8192.0f, 1.0f/4096.0f, 0.0f);
	uniform vec3 texOffsetDetail = vec3(1.0f/800.0f, 1.0f/800.0f, 0.0f);

	out vec4 outColor;
	
	float Lambert(vec3 norm, vec3 lightDir)
	{
		return max(dot(norm, -lightDir), 0);
	}
	vec3 DirLighting(vec3 dif, vec3 norm)
	{
		vec3 diffuse = (dif) * Lambert(norm, lightDir);
		//vec3 specular = (spec * light.Color) * Blinn(norm, light.Direction, viewDir, specPow);
		return diffuse;// + specular;
	}
	float height(vec2 uv, sampler2D tex)
	{
		return texture(tex, uv).r*maxHeight;
	}
	vec3 CalculateNormal(vec2 uv, sampler2D tex, vec3 texOffset)
	{
		// read neightbor heights using an arbitrary small offset
		float hL = height(uv - texOffset.xz, tex);
		float hR = height(uv + texOffset.xz, tex);
		float hD = height(uv - texOffset.zy, tex);
		float hU = height(uv + texOffset.zy, tex);
		// deduce terrain normal
		vec3 N = normalize(vec3(hL - hR, hD - hU, 2.0));
		vec3 norm = normalize(Normal);
		vec3 up = vec3(0, 1, 0)-norm;
		vec3 tang = normalize(cross(norm, up));//might need flipping
		vec3 biTan = normalize(cross(norm, tang));//same
		mat3 localAxis = mat3(tang, biTan, norm);
		return normalize(localAxis * normalize(N));
	}
	void main()
	{
		vec3 tc3 = normalize(Tex3);
		vec2 uv;
		uv.x = atan( tc3.z, tc3.x );
		uv.y = acos( tc3.y );
		uv /= vec2( 2.0f * 3.14159265359f, 3.14159265359f );
		vec3 dif = texture(texDiffuse, uv).rgb;
		
		vec2 tileStretch = vec2(2, 1);
		vec2 detailUV = uv*tileStretch*100;
		vec3 detail1 = texture(texDetail1, detailUV).rgb;
		vec3 detail2 = texture(texDetail2, uv*tileStretch*700).rgb;
		
		float dist = length(TriPos-camPos);
		vec3 mix1 = detail1*dif;
		float craterAmount = 1-clamp(dif.r-0.5f, 0, 1);
		float detail = craterAmount*clamp((dist-1000)/3000, 0, 1);
		dif = mix(mix1, dif, detail);
		vec3 mix2 = detail2*dif;
		dif = mix(mix2, dif, clamp((dist-1000)/2000, 0, 1));
		
		//Calculate normal
		vec3 norm = CalculateNormal(uv, texHeight, texOffset);
		vec3 normDetail = CalculateNormal(detailUV, texHeightDetail, texOffsetDetail);
		norm = normalize(norm+normDetail*detail*2);
	
		vec3 lit = DirLighting(dif, norm);
		lit*=0.8f;
		outColor = vec4(lit+ambient*dif, 1.0f);
	} 
</FRAGMENT>