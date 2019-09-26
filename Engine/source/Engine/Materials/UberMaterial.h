#pragma once
#include <Engine/Graphics/Material.h>


class TextureData;


class UberMaterial : public Material
{
public:
	UberMaterial();
	virtual ~UberMaterial() = default;

	void SetBaseColorTexture(T_Hash const id);
	void SetNormalTexture(T_Hash const id);
	void SetMetallicRoughnessTexture(T_Hash const id);

	void SetBaseColor(vec3 col) { m_BaseColor = col; }
	void SetRoughness(float val) { m_Roughness = val; }
	void SetMetallic(float val) { m_Metallic = val; }

private:
	void LoadTextures() override;
	void AccessShaderAttributes() override {}
	void UploadDerivedVariables() override;

private:
	//Texture
	AssetPtr<TextureData> m_TexBaseColor;
	T_Hash m_TexBaseColorAsset = 0u;

	AssetPtr<TextureData> m_TexNormal;
	T_Hash m_TexNormalAsset = 0u;

	AssetPtr<TextureData> m_TexMetallicRoughness;
	T_Hash m_TexMetallicRoughnessAsset = 0u;

	bool m_OutdatedTextureData = false;

	//Phong parameters
	vec3 m_BaseColor;
	float m_Roughness;
	float m_Metallic = 50;
};

