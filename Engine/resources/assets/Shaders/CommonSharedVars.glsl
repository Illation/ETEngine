#extension GL_ARB_bindless_texture : require

//Some constants
layout(std140) uniform SharedVars
{
	mat4 view;
	mat4 viewInv;
	mat4 projection;
	mat4 viewProjection;
	mat4 viewProjectionInv;
	mat4 staticViewProjection; // rotation and scewing without translation
	mat4 staticViewProjectionInv; 

	float time;
	float deltaTime;

	float projectionA;
	float projectionB;

	vec3 camPos;

	sampler2D texGBufferA;		/*| <----   Depth   ----> | xxxxx |*/
	sampler2D texGBufferB;  	/*| Nor.x   Nor.y | Met.x | AO .x |*/
	sampler2D texGBufferC; 		/*| BCo.r   BCo.g   BCo.b | Rou.x |*/
};