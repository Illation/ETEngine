#pragma once
#include "../../Graphics/Material.hpp"

class TextureData;
class DiffuseMaterial : public Material
{
public:
	DiffuseMaterial();
	DiffuseMaterial(std::string texDiffusePath);
	~DiffuseMaterial();

	void SetLightDir(glm::vec3 lightDir) { m_LightDir = lightDir; }
	void SetAmbCol(glm::vec3 col) { m_AmbientColor = col; }
	void SetDifCol(glm::vec3 col) { m_DiffuseColor = col; }
	void SetSpecCol(glm::vec3 col) { m_SpecularColor = col; }
	void SetSpecPow(float pow) { m_SpecularPower = pow; }
private:
	void LoadTextures();
	void AccessShaderAttributes();

	void UploadDerivedVariables();

private:
	//Texture
	GLint m_uUseDifTex;
	bool m_UseDifTex = false;
	TextureData* m_TexDiffuse;
	std::string m_TexDiffusePath;
	//Light
	GLint m_uLightDir;
	glm::vec3 m_LightDir;
	//CameraPos
	GLint m_uCamPos;
	//Phong parameters
	GLint m_uAmbCol;
	glm::vec3 m_AmbientColor;
	GLint m_uDifCol;
	glm::vec3 m_DiffuseColor;
	GLint m_uSpecCol;
	glm::vec3 m_SpecularColor;
	GLint m_uSpecPow;
	float m_SpecularPower = 50;
};

