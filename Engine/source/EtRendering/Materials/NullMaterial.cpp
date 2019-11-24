#include "stdafx.h"
#include "NullMaterial.h"


NullMaterial::NullMaterial()
	: Material("Shaders/FwdNullShader.glsl")
{
	m_DrawForward = true;
}