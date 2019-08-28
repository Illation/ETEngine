#pragma once
#include <Engine/Graphics/Material.h>


class LightMaterial : public Material
{
public:
	LightMaterial(vec3 col = vec3(1, 1, 1));
	virtual ~LightMaterial() = default;

	void SetLight(vec3 pos, vec3 color, float radius)
	{
		m_Position = pos;
		m_Color = color;
		m_Radius = radius;
	}
private:
	void LoadTextures() override {}
	void AccessShaderAttributes() override;
	void UploadDerivedVariables() override;

private:
	//Parameters
	GLint m_uCol;
	vec3 m_Color;
	GLint m_uPosition;
	vec3 m_Position;
	GLint m_uRadius;
	float m_Radius;

	GLint m_uCamPos;

	GLint m_uProjA;
	GLint m_uProjB;
	GLint m_uViewProjInv;
};

