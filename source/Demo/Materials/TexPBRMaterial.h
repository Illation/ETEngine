#pragma once
#include "../../Engine/Graphics/Material.hpp"

class TextureData;
class TexPBRMaterial : public Material
{
public:
	TexPBRMaterial(std::string bcPath, std::string roughPath,
		std::string metalPath, std::string aoPath, std::string normPath);
	~TexPBRMaterial();

	void SetSpecular(float spec) { m_Specular = spec; }
private:
	void LoadTextures();
	void AccessShaderAttributes();

	void UploadDerivedVariables();

private:
	//Texture
	TextureData* m_TexBaseColor = nullptr;
	std::string m_TexBCPath;
	TextureData* m_TexRoughness = nullptr;
	std::string m_TexRoughPath;
	TextureData* m_TexMetalness = nullptr;
	std::string m_TexMetalPath;
	TextureData* m_TexAO = nullptr;
	std::string m_TexAOPath;
	TextureData* m_TexNorm = nullptr;
	std::string m_TexNormPath;

	bool m_OutdatedTextureData = false;

	//Phong parameters
	GLint m_uSpecular;
	float m_Specular = 0.5;
};

