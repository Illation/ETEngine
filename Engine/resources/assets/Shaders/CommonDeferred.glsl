
vec2 encodeNormal(vec3 norm)
{
	float scale = 1.7777;
	vec2 enc = norm.xy / (norm.z+1);
	enc /= scale;
	return (enc*0.5)+0.5;
}
vec3 decodeNormal(vec2 enc)
{
	float scale = 1.7777;
	vec3 nn =
		vec3(enc, 0)*vec3(2*scale,2*scale,0) +
		vec3(-scale,-scale,1);
	float g = 2.0 / dot(nn.rgb,nn.rgb);
	return vec3(g*nn.xy, g-1);
}

vec3 reconstructPosition(vec3 viewRay, vec3 camPosition, float depth, float projA, float projB)
{
	float linearDepth = projB / (depth - projA);
	return camPosition + viewRay * linearDepth;
}

#include "Shaders/CommonSharedVars.glsl"

/*| <----   Depth   ----> | xxxxx |*/
/*| Nor.x   Nor.y | Met.x | AO .x |*/
/*| BCo.r   BCo.g   BCo.b | Rou.x |*/

#define UNPACK_DEPTH(texCoord, texGBufferA)	\
texture(texGBufferA, texCoord).r			\

#define LINEAR_DEPTH(inDepth)			\
projectionB / (inDepth - projectionA)	\

#define UNPACK_GBUFFER(texCoord, viewRay, texGBufferA, texGBufferB, texGBufferC) 											\
float depth = texture(texGBufferA, texCoord).r;										\
vec3 pos = reconstructPosition(viewRay, camPos, depth, projectionA, projectionB); 	\
vec3 norm = decodeNormal(texture(texGBufferB, texCoord).rg); 						\
vec3 baseCol = texture(texGBufferC, texCoord).rgb; 									\
float rough = texture(texGBufferC, texCoord).a; 									\
float metal = texture(texGBufferB, texCoord).b; 									\
float ao = texture(texGBufferB, texCoord).a; 										\
float spec = 0.5f;																	\