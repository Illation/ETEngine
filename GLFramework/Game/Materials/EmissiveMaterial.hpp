#pragma once
#include "../../Graphics/Material.hpp"

class EmissiveMaterial : public Material
{
public:
	EmissiveMaterial(glm::vec3 col = glm::vec3(1, 1, 1));
	~EmissiveMaterial();

	void SetCol(glm::vec3 col) { m_Color = col; }
private:
	void LoadTextures();
	void AccessShaderAttributes();

	void UploadDerivedVariables();

private:
	//Parameters
	GLint m_uCol;
	glm::vec3 m_Color;
};

