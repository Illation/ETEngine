#include "Material.hpp"

#include "../Content/ShaderLoader.hpp"
#include "../Base\Context.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "../Content/ContentManager.hpp"
#include "ShaderData.hpp"

#define CAMERA Context::GetInstance()->pCamera

Material::Material(std::string shaderFile) :
	m_ShaderFile(shaderFile)
{
}
Material::~Material()
{
}

void Material::Initialize()
{
	//Load Shader
	m_Shader = ContentManager::Load<ShaderData>(m_ShaderFile);

	//Jup (maybe temporary with texture manager)
	LoadTextures();

	//Get Access to uniforms
	m_UniMatModel = glGetUniformLocation(m_Shader->GetProgram(), "model");
	m_UniMatWVP = glGetUniformLocation(m_Shader->GetProgram(), "worldViewProj");
	AccessShaderAttributes();
}

void Material::UploadVariables(glm::mat4 matModel)
{
	glUseProgram(m_Shader->GetProgram());
	//Upload matrices
	glUniformMatrix4fv(m_UniMatModel, 1, GL_FALSE, glm::value_ptr(matModel));
	glUniformMatrix4fv(m_UniMatWVP, 1, GL_FALSE, glm::value_ptr(CAMERA->GetViewProj()));

	UploadDerivedVariables();
}
