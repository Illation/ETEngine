#include <Engine/stdafx.h>
#include "IdMaterial.h"
#include <Engine/Graphics/Shader.h>


//---------------------------------
// IdMaterial::c-tor
//
// init with DebugRenderer shader
//
IdMaterial::IdMaterial()
	: Material("FwdIdShader.glsl")
{
	m_DrawForward = true;
}