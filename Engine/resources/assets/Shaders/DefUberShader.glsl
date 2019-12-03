<VERTEX>
	#version 330 core
	#include "CommonSharedVars.glsl"
	
	in vec3 position;
	in vec3 normal;
	in vec3 tangent;
	in vec2 texcoord;
	
	out vec3 Position;
	out vec3 Normal;
	out vec3 Tangent;
	out vec2 Texcoord;
	
	uniform mat4 model;
	
	void main()
	{
		Texcoord = texcoord;
		
		mat3 normMat = inverse(mat3(model));
		normMat = transpose(normMat);
		Normal = normalize(normMat*normal);
		Tangent = normalize(normMat*tangent);
		
		vec4 pos = model*vec4(position, 1.0);
		Position = vec3(pos.x, pos.y, pos.z);
		gl_Position = viewProjection*pos;
	}
</VERTEX>
<FRAGMENT>
  #version 330 core

  #include "CommonDeferred.glsl"

  in vec3 Position;
  in vec3 Normal;
  in vec3 Tangent;
  in vec2 Texcoord;

  layout(location = 0) out vec4 outGBufferB;
  layout(location = 1) out vec4 outGBufferC;

  uniform sampler2D uTexBaseColor;
  uniform sampler2D uTexNormal;
  uniform sampler2D uTexMetallicRoughness;
  uniform sampler2D uTexOcclusion;
  uniform sampler2D uTexEmissive;

  uniform bool uUseBaseColTex;
  uniform bool uUseNormalTex;
  uniform bool uUseMetallicRoughnessTex;
  uniform bool uUseOcclusionTex;
  uniform bool uUseEmissiveTex;

  uniform vec3 uBaseColor = vec3(1.f);
  uniform vec3 uEmissiveFactor = vec3(0.f);
  uniform float uRoughness = 0.f;
  uniform float uMetallic = 0.f;


  vec3 mapNormal()
  {
    vec3 norm = normalize(Normal);

    if (!uUseNormalTex)
    {
      return norm;
    }

    vec3 tang = normalize(Tangent);
    vec3 binorm = normalize(cross(tang, norm));
    mat3 localAxis = mat3(tang, binorm, norm);

    vec3 normSample = ((texture(uTexNormal, Texcoord).rgb)*2)-vec3(1, 1, 1);
    norm = localAxis*normalize(normSample);
    return normalize(norm);
  }

  void main()
  {
    vec3 norm = mapNormal();

    vec3 baseCol = uBaseColor;
    if (uUseBaseColTex)
    {
      baseCol *= texture(uTexBaseColor, Texcoord).rgb;
    }

    vec3 emissive = uEmissiveFactor;
    if (uUseEmissiveTex)
    {
      emissive *= texture(uTexEmissive, Texcoord).rgb;
    }

    baseCol += emissive;

    float rough = uRoughness;
    float metal = uMetallic;
    if (uUseMetallicRoughnessTex)
    {
      rough *= texture(uTexMetallicRoughness, Texcoord).g;
      metal *= texture(uTexMetallicRoughness, Texcoord).b;
    }

    float ao = 1.f;
    if (uUseOcclusionTex)
    {
      ao *= texture(uTexOcclusion, Texcoord).r;
    }

	outGBufferB = vec4(encodeNormal(norm), metal, ao);
    outGBufferC = vec4(baseCol, rough);
  }
</FRAGMENT>