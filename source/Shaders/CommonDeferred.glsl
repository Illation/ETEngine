
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

#define GBUFFER_SAMPLER														\
uniform sampler2D texPosAO;        /*| Pos.x   Pos.y   Pos.z | AO .x |*/ 	\
uniform sampler2D texNormMetSpec;  /*| Nor.x   Nor.y | Met.x | Spc.x |*/ 	\
uniform sampler2D texBaseColRough; /*| BCo.r   BCo.g   BCo.b | Rou.x |*/ 	\

#define UNPACK_GBUFFER(texCoord) 								\
vec3 pos = texture(texPosAO, texCoord).rgb; 					\
vec3 norm = decodeNormal(texture(texNormMetSpec, texCoord).rg); \
vec3 baseCol = texture(texBaseColRough, texCoord).rgb; 			\
float rough = texture(texBaseColRough, texCoord).a; 			\
float metal = texture(texNormMetSpec, texCoord).b; 				\
float ao = texture(texPosAO, texCoord).a; 						\
float spec = texture(texNormMetSpec, texCoord).a; 				\