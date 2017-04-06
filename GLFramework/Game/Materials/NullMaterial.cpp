#include "stdafx.hpp"
#include "NullMaterial.hpp"
#include "../../Graphics/ShaderData.hpp"
#include "../../Graphics/MeshFilter.hpp"


NullMaterial::NullMaterial():
	Material("Resources/Shaders/FwdLightShader.glsl")
{
	m_LayoutFlags = VertexFlags::POSITION;
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