#pragma once
#include "../../Graphics/Material.hpp"

class LightMaterial : public Material
{
public:
	LightMaterial(glm::vec3 col = glm::vec3(1, 1, 1));
	~LightMaterial();

	void SetLight(glm::vec3 pos, glm::vec3 color, float radius)
	{
		m_Position = pos;
		m_Color = color;
		m_Radius = radius;
	}
private:
	void LoadTextures();
	void AccessShaderAttributes();

	void UploadDerivedVariables();

private:
	//Parameters
	GLint m_uCol;
	glm::vec3 m_Color;
	GLint m_uPosition;
	glm::vec3 m_Position;
	GLint m_uRadius;
	float m_Radius;

	GLint m_uCamPos;

};

