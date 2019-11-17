#include "stdafx.h"
#include "EmissiveMaterial.h"

#include <EtRendering/GraphicsTypes/Shader.h>


EmissiveMaterial::EmissiveMaterial(vec3 col)
	: Material("Shaders/FwdEmissiveShader.glsl")
	, m_Color(col)
{
	m_DrawForward = true;
}

void EmissiveMaterial::UploadDerivedVariables()
{
	m_Shader->Upload("color"_hash, m_Color);
}