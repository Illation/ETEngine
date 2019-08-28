#include "stdafx.h"
#include "NullMaterial.h"
#include <Engine/Graphics/Shader.h>
#include <Engine/Graphics/MeshFilter.h>


NullMaterial::NullMaterial()
	: Material("Shaders/FwdNullShader.glsl")
{
	m_DrawForward = true;
}