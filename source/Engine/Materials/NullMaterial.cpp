#include "stdafx.h"
#include "NullMaterial.h"
#include <Engine/Graphics/Shader.h>
#include <Engine/Graphics/MeshFilter.h>


NullMaterial::NullMaterial():
	Material("Shaders/FwdNullShader.glsl")
{
	m_LayoutFlags = E_VertexFlag::POSITION;
	m_DrawForward = true;
}
NullMaterial::~NullMaterial()
{
}

void NullMaterial::LoadTextures()
{
}
void NullMaterial::AccessShaderAttributes()
{
}
void NullMaterial::UploadDerivedVariables()
{
}