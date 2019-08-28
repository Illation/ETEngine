#pragma once
#include <Engine/Graphics/Material.h>


class EmissiveMaterial final : public Material
{
public:
	EmissiveMaterial(vec3 col = vec3(1, 1, 1));
	virtual ~EmissiveMaterial() = default;

	void SetCol(vec3 col) { m_Color = col; }
private:
	void LoadTextures() override {}
	void AccessShaderAttributes() override;

	void UploadDerivedVariables() override;

private:
	//Parameters
	GLint m_uCol;
	vec3 m_Color;
};

