#include "stdafx.h"
#include "ColorMaterial.h"


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