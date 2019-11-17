#pragma once
#include <EtRendering/GraphicsTypes/Material.h>


class TextureData;


class UberMaterial : public Material
{
public:
	UberMaterial();
	virtual ~UberMaterial() = default;

	void SetBaseColorTexture(T_Hash const id);
	void SetNormalTexture(T_Hash const id);
	void SetMetallicRoughnessTexture(T_Hash const id);
	void SetOcclusionTexture(T_Hash const id);
	void SetEmissiveTexture(T_Hash const id);

	void SetBaseColor(vec3 col) { m_BaseColor = col; }
	void SetEmissiveFactor(vec3 fac) { m_EmissiveFactor = fac; }
	void SetRoughness(float val) { m_Roughness = val; }
	void SetMetallic(float val) { m_Metallic = val; }

private:
	void LoadTextures() override;
	void AccessShaderAttributes() override {}
	void UploadDerivedVariables() override;

private:
	AssetPtr<TextureData> m_TexBaseColor;
	T_Hash m_TexBaseColorAsset = 0u;

	AssetPtr<TextureData> m_TexNormal;
	T_Hash m_TexNormalAsset = 0u;

	AssetPtr<TextureData> m_TexMetallicRoughness;
	T_Hash m_TexMetallicRoughnessAsset = 0u;

	AssetPtr<TextureData> m_TexOcclusion;
	T_Hash m_TexOcclusionAsset = 0u;

	AssetPtr<TextureData> m_TexEmissive;
	T_Hash m_TexEmissiveAsset = 0u;

	vec3 m_BaseColor = vec3(1.f);
	vec3 m_EmissiveFactor = vec3(0.f);
	float m_Roughness = 1.f;
	float m_Metallic = 1.f;

	// should also be able to specify alpha mode, cutoff, and double sidedness

	bool m_OutdatedTextureData = false;
};

