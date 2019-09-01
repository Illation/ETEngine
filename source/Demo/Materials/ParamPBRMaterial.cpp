#include "stdafx.h"
#include "ParamPBRMaterial.h"

#include <Engine/Graphics/Shader.h>


ParamPBRMaterial::ParamPBRMaterial(vec3 baseCol, float roughness, float metal) 
	: Material("Shaders/DefPBRParamShader.glsl")
	, m_BaseColor(baseCol)
	, m_Roughness(roughness)
	, m_Metal(metal)
{}

void ParamPBRMaterial::UploadDerivedVariables()
{
	m_Shader->Upload("baseColor"_hash, m_BaseColor);
	m_Shader->Upload("roughness"_hash, m_Roughness);
	m_Shader->Upload("metalness"_hash, m_Metal);
}