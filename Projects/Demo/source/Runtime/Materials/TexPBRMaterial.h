#pragma once
#include <EtRendering/GraphicsTypes/Material.h>


class TextureData;


class TexPBRMaterial : public Material
{
public:
	TexPBRMaterial(T_Hash const bcId, T_Hash const roughId, T_Hash const metalId, T_Hash const aoId, T_Hash const normId);
	virtual ~TexPBRMaterial() = default;

	void SetSpecular(float spec) { m_Specular = spec; }

private:
	void LoadTextures() override;
	void AccessShaderAttributes() override {}
	void UploadDerivedVariables() override;

private:
	//Texture
	AssetPtr<TextureData> m_TexBaseColor;
	T_Hash m_BaseColorId;

	AssetPtr<TextureData> m_TexRoughness;
	T_Hash m_RoughnessId;

	AssetPtr<TextureData> m_TexMetalness;
	T_Hash m_MetalnessId;

	AssetPtr<TextureData> m_TexAO;
	T_Hash m_AoId;

	AssetPtr<TextureData> m_TexNorm;
	T_Hash m_NormalId;

	bool m_OutdatedTextureData = false;

	//Phong parameters
	float m_Specular = 0.5;
};

