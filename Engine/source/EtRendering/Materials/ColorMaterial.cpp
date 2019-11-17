#include "stdafx.h"
#include "ColorMaterial.h"
#include <Engine/Graphics/Shader.h>


//---------------------------------
// ColorMaterial::c-tor
//
// init with DebugRenderer shader
//
ColorMaterial::ColorMaterial()
	: Material("FwdColorShader.glsl")
{
	m_DrawForward = true;
}