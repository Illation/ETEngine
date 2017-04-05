#pragma once
#include "../../Graphics/Material.hpp"

class LightMaterial : public Material
{
public:
	LightMaterial(glm::vec3 col = glm::vec3(1, 1, 1));
	~LightMaterial();
private:
	void LoadTextures();
	void AccessShaderAttributes();

	void UploadDerivedVariables();

private:
	//Parameters
	GLint m_uCol;
	glm::vec3 m_Color;
};

