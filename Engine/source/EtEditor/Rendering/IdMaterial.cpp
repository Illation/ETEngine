#include <EtFramework/stdafx.h>
#include "IdMaterial.h"


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