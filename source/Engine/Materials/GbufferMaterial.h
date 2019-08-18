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
	void LoadTextures();
	void AccessShaderAttributes();

	void UploadDerivedVariables();

private:
	//Texture
	GLint m_uUseDifTex;
	AssetPtr<TextureData> m_TexDiffuse;
	T_Hash m_TexDiffuseAsset = 0u;

	GLint m_uUseNormTex;
	AssetPtr<TextureData> m_TexNorm;
	T_Hash m_TexNormAsset = 0u;

	GLint m_uUseSpecTex;
	AssetPtr<TextureData> m_TexSpec;
	T_Hash m_TexSpecAsset = 0u;

	bool m_OutdatedTextureData = false;

	//Phong parameters
	GLint m_uDifCol;
	vec3 m_DiffuseColor;
	GLint m_uSpecCol;
	vec3 m_SpecularColor;
	GLint m_uSpecPow;
	float m_SpecularPower = 50;
};

