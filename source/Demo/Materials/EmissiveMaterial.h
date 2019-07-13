#pragma once
#include "../../Engine/Graphics/Material.hpp"

class EmissiveMaterial : public Material
{
public:
	EmissiveMaterial(vec3 col = vec3(1, 1, 1));
	~EmissiveMaterial();

	void SetCol(vec3 col) { m_Color = col; }
private:
	void LoadTextures();
	void AccessShaderAttributes();

	void UploadDerivedVariables();

private:
	//Parameters
	GLint m_uCol;
	vec3 m_Color;
};

