#pragma once
#include "../Graphics/Material.hpp"

class TextureData;
class GbufferMaterial : public Material
{
public:
	GbufferMaterial();
	~GbufferMaterial();

	void SetDiffuseTexture(std::string path) 
	{ 
		m_TexDiffusePath = path; 
		m_UseDifTex = true;
		m_OutdatedTextureData = true;
	}
	void SetNormalTexture(std::string path) 
	{
		m_TexNormPath = path; 
		m_UseNormTex = true;
		m_OutdatedTextureData = true;
	}
	void SetSpecularTexture(std::string path)
	{
		m_TexSpecPath = path;
		m_UseSpecTex = true;
		m_OutdatedTextureData = true;
	}

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
	bool m_UseDifTex = false;
	TextureData* m_TexDiffuse = nullptr;
	std::string m_TexDiffusePath;

	GLint m_uUseNormTex;
	bool m_UseNormTex = false;
	TextureData* m_TexNorm = nullptr;
	std::string m_TexNormPath;

	GLint m_uUseSpecTex;
	bool m_UseSpecTex = false;
	TextureData* m_TexSpec = nullptr;
	std::string m_TexSpecPath;

	bool m_OutdatedTextureData = false;

	//Phong parameters
	GLint m_uDifCol;
	vec3 m_DiffuseColor;
	GLint m_uSpecCol;
	vec3 m_SpecularColor;
	GLint m_uSpecPow;
	float m_SpecularPower = 50;
};

