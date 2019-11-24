#pragma once
#include <EtRendering/GraphicsTypes/Material.h>


class TextureData;
class ParamPBRMaterial : public Material
{
public:
	ParamPBRMaterial(vec3 baseCol, float roughness, float metal);
	virtual ~ParamPBRMaterial() = default;

private:
	void LoadTextures() override {}
	void AccessShaderAttributes() override {}
	void UploadDerivedVariables() override;

private:

	vec3 m_BaseColor = vec3(0.8f);
	float m_Roughness = 0.5f;
	float m_Metal = 0;
};

