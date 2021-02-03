<VERTEX>
	#version 330 core
	in vec2 position;
	in vec2 texcoord;
	out vec2 Texcoord;
	void main()
	{
		Texcoord = texcoord;
		gl_Position = vec4(position, 0.0, 1.0);
	}
</VERTEX>
<FRAGMENT>
	#version 330 core
	
	#include "Shaders/Common.glsl"
	#include "Shaders/CommonSharedVars.glsl"
	
	in vec2 Texcoord;
	
	layout (location = 0) out vec4 outColor;
	layout (location = 1) out vec4 brightColor;
	
	uniform sampler2D texPosAO;                   // | Pos.x   Pos.y   Pos.z | AO .x |
	uniform sampler2D texNormMetSpec;             // | Nor.x   Nor.y | Met.x | Spc.x |
	uniform sampler2D texBaseColRough;            // | BCo.r   BCo.g   BCo.b | Rou.x |
	
	uniform samplerCube texEnvironment;
	
	uniform float fresnelPow = 2.5;
	uniform float fresnelMult = 2.0;
	uniform float fresnelHard = 1.0;
	uniform vec3 fresnelCol = vec3(1, 1, 1);
	uniform vec3 fresnelUp = vec3(0, 1, 0);
	
	struct PointLight 
	{
		vec3 Position;
		vec3 Color;
		
		float Radius;
	};
	struct DirectionalLight
	{
		vec3 Direction;
		vec3 Color;
	};
	const int NR_POINT_LIGHTS = 1;
	uniform PointLight pointLights[NR_POINT_LIGHTS];
	const int NR_DIR_LIGHTS = 1;
	uniform DirectionalLight dirLights[NR_DIR_LIGHTS];
	
	
	vec3 decodeNormal(vec2 enc)
	{
		float scale = 1.7777;
		vec3 nn =
			vec3(enc, 0)*vec3(2*scale,2*scale,0) +
			vec3(-scale,-scale,1);
		float g = 2.0 / dot(nn.rgb,nn.rgb);
		return vec3(g*nn.xy, g-1);
	}
	
	void main()
	{
		int tileRows = 3;
		int tileX;
		int tileY;
		float increment = 1.0 / float(tileRows);
		for(int i = 0; i < tileRows; i++)
		{
			if(((i)*increment < Texcoord.x) && (Texcoord.x < (i+1)*increment))tileX = i;
			if(((i)*increment < Texcoord.y) && (Texcoord.y < (i+1)*increment))tileY = i;
		}
		vec2 tc = fract(vec2(Texcoord.x*tileRows, Texcoord.y*tileRows));
		vec3 finalCol;// = vec3(tc.x, tc.y, float(tileX)/float(tileRows-1));
		
		float alpha = 1.0;
		vec3 vecAlpha = vec3(alpha, alpha, alpha);
		
		if(tileX == 0 && tileY == 2)//Position
		{
			finalCol = texture(texPosAO, tc).rgb;
		}
		else if(tileX == 1 && tileY == 2)//normal
		{
			finalCol = decodeNormal(texture(texNormMetSpec, tc).rg);
			finalCol += vecAlpha;
			finalCol *= 0.5;
		}
		else if(tileX == 2 && tileY == 2)//base color
		{
			finalCol = texture(texBaseColRough, tc).rgb;
		}
		else if(tileX == 0 && tileY == 1)//roughness
		{
			finalCol = vecAlpha* texture(texBaseColRough, tc).a;
			
		}
		else if(tileX == 1 && tileY == 1)//metalness
		{
			finalCol = vecAlpha* texture(texNormMetSpec, tc).b;
		}
		else if(tileX == 2 && tileY == 1)//ao
		{
			finalCol = vecAlpha* texture(texPosAO, tc).a;
		}
		else if(tileX == 0 && tileY == 0)//specular
		{
			finalCol = vecAlpha* texture(texNormMetSpec, tc).a;
		}
		else if(tileX == 1 && tileY == 0)//reflection
		{
			float rough = texture(texBaseColRough, tc).a;
			vec3 pos = texture(texPosAO, tc).rgb;
			vec3 norm = decodeNormal(texture(texNormMetSpec, tc).rg);
			vec3 viewDir = normalize(pos - camPos);
			vec3 refl = reflect(viewDir, norm);
			vec3 flipRef = refl * vec3(1, -1, 1);
			finalCol = textureLod(texEnvironment, flipRef, rough*11.0).rgb;
		}
		else if(tileX == 2 && tileY == 0)//alpha
		{
			vec3 pos = texture(texPosAO, tc).rgb;
			vec3 norm = decodeNormal(texture(texNormMetSpec, tc).rg);
			vec3 viewDir = normalize(pos - camPos);
			
			float fresnel = abs(dot(norm, viewDir));
			fresnel = 1 - clamp(fresnel, 0.0, 1.0);
			fresnel = pow(fresnel,fresnelPow)*fresnelMult;
		
			float fresnelMask = dot(fresnelUp,norm);
			fresnelMask = clamp(fresnelMask, 0.0, 1.0);
			fresnelMask = pow(1 - fresnelMask,fresnelHard);
			fresnel *= fresnelMask;
			
			finalCol = vecAlpha*fresnel;
		}
	
		outColor = vec4(finalCol, alpha);
		brightColor = vec4(0, 0, 0, 1.0);
	}
</FRAGMENT>