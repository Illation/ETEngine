<VERTEX>
	#version 330 core
	
	in vec3 position;
	in vec3 normal;
	in vec3 tangent;
	in vec2 texcoord;
	
	out vec3 Position;
	out vec3 Normal;
	out vec3 Tangent;
	out vec2 Texcoord;
	
	uniform mat4 model;
	uniform mat4 worldViewProj;
	
	void main()
	{
		Texcoord = texcoord;
		
		mat3 normMat = inverse(mat3(model));
		normMat = transpose(normMat);
		Normal = normalize(normMat*normal);
		Tangent = normalize(normMat*tangent);
		
		vec4 pos = model*vec4(position, 1.0);
		Position = vec3(pos.x, pos.y, pos.z);
		gl_Position = worldViewProj*pos;
	}
</VERTEX>
<FRAGMENT>
  #version 330 core

  #include "CommonDeferred.glsl"

  in vec3 Position;
  in vec3 Normal;
  in vec3 Tangent;
  in vec2 Texcoord;

  layout (location = 0) out vec4 texGBufferB;
  layout (location = 1) out vec4 texGBufferC;

  uniform sampler2D texDiffuse;
  uniform sampler2D texSpecular;
  uniform sampler2D texNormal;

  uniform bool useDifTex;
  uniform bool useNormTex;
  uniform bool useSpecTex;

  uniform vec3 diffuseColor = vec3(1);
  uniform vec3 specularColor = vec3(1);
  uniform float specularPower = 1;


  vec3 mapNormal()
  {
    vec3 norm = normalize(Normal);

    if (!useNormTex)
    {
      return norm;
    }

    vec3 tang = normalize(Tangent);
    vec3 binorm = normalize(cross(tang, norm));
    mat3 localAxis = mat3(tang, binorm, norm);

    vec3 normSample = ((texture(texNormal, Texcoord).rgb)*2)-vec3(1, 1, 1);
    norm = localAxis*normalize(normSample);
    return normalize(norm);
  }

  void main()
  {
    vec3 norm = mapNormal();

    vec3 baseCol = diffuseColor;
    if (useDifTex)
    {
      baseCol *= texture(texDiffuse, Texcoord).rgb;
    }

    float rough = 1.f;
    float metal = specularPower;
    if (useSpecTex)
    {
      rough *= texture(texSpecular, Texcoord).g;
      metal *= texture(texSpecular, Texcoord).b;
    }

    float ao = 1.f;

    texGBufferB = vec4(encodeNormal(norm), metal, ao);
    texGBufferC = vec4(baseCol, rough);
  }
</FRAGMENT>