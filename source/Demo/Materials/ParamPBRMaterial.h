#pragma once
#include "../../Engine/Graphics/Material.hpp"

class TextureData;
class ParamPBRMaterial : public Material
{
public:
	ParamPBRMaterial(vec3 baseCol, float roughness, float metal);
	~ParamPBRMaterial();

private:
	void LoadTextures();
	void AccessShaderAttributes();

	void UploadDerivedVariables();

private:

	GLint m_uBaseColor;
	vec3 m_BaseColor = vec3(0.8f);
	GLint m_uRoughness;
	float m_Roughness = 0.5f;
	GLint m_uMetal;
	float m_Metal = 0;
};

