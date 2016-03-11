#pragma once
#include "../../Graphics/Material.hpp"
class CubeMaterial :
	public Material
{
public:
	CubeMaterial(std::string texKittenPath, std::string texPuppyPath);
	~CubeMaterial();

	void SpecifyInputLayout();
	void UpdateReflectionAtt(glm::mat4 model, glm::vec3 col);
private:
	void LoadTextures();
	void AccessShaderAttributes();

	void UploadDerivedVariables();
private:
	//Textures
	GLuint m_TexKitten;
	GLuint m_TexPuppy;
	std::string m_TexKittenPath;
	std::string m_TexPuppyPath;
	//Shader uniforms
	GLint m_UniTime;
	GLint m_UniColor;
};

