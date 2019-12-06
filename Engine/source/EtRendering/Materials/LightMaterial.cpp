#include "stdafx.h"
#include "LightMaterial.h"

#include <EtRendering/GraphicsTypes/Shader.h>


LightMaterial::LightMaterial(vec3 col)
	: Material("Shaders/FwdLightPointShader.glsl")
	, m_Color(col)
{
	m_DrawForward = true;
}

void LightMaterial::SetLight(vec3 pos, vec3 color, float radius)
{
	m_Position = pos;
	m_Color = color;
	m_Radius = radius;
}

void LightMaterial::UploadDerivedVariables()
{
	m_Shader->Upload("Position"_hash, m_Position);
	m_Shader->Upload("Color"_hash, m_Color);
	m_Shader->Upload("Radius"_hash, m_Radius);
}