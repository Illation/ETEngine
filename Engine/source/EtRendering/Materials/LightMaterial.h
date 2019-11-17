#pragma once
#include <Engine/Graphics/Material.h>


class LightMaterial : public Material
{
public:
	LightMaterial(vec3 col = vec3(1, 1, 1));
	virtual ~LightMaterial() = default;

	void SetLight(vec3 pos, vec3 color, float radius);
private:
	void LoadTextures() override {}
	void AccessShaderAttributes() override {}
	void UploadDerivedVariables() override;

private:
	//Parameters
	vec3 m_Color;
	vec3 m_Position;
	float m_Radius;
};

