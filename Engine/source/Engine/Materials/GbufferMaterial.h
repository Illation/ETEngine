#pragma once
#include <Engine/Graphics/Material.h>


class TextureData;
class GbufferMaterial : public Material
{
public:
	GbufferMaterial();
	virtual ~GbufferMaterial() = default;

	void SetDiffuseTexture(T_Hash const id);
	void SetNormalTexture(T_Hash const id);
	void SetSpecularTexture(T_Hash const id);

	void SetDifCol(vec3 col) { m_DiffuseColor = col; }
	void SetSpecCol(vec3 col) { m_SpecularColor = col; }
	void SetSpecPow(float pow) { m_SpecularPower = pow; }

private:
	void LoadTextures() override;
	void AccessShaderAttributes() override {}
	void UploadDerivedVariables() override;

private:
	//Texture
	AssetPtr<TextureData> m_TexDiffuse;
	T_Hash m_TexDiffuseAsset = 0u;

	AssetPtr<TextureData> m_TexNorm;
	T_Hash m_TexNormAsset = 0u;

	AssetPtr<TextureData> m_TexSpec;
	T_Hash m_TexSpecAsset = 0u;

	bool m_OutdatedTextureData = false;

	//Phong parameters
	vec3 m_DiffuseColor;
	vec3 m_SpecularColor;
	float m_SpecularPower = 50;
};

