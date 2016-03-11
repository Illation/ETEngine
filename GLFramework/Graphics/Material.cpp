#include "Material.hpp"

#include "../Content/ShaderLoader.hpp"
#include "../Base\Context.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#define CAMERA Context::GetInstance()->pCamera

Material::Material(std::string VSfile, std::string FSfile, std::string GSfile, bool UseGeometryShader) :
	m_VSfile(VSfile),
	m_GSfile(GSfile),
	m_FSfile(FSfile),
	m_UseGS(UseGeometryShader)
{
}
Material::~Material()
{
	glDeleteProgram(m_ShaderProgram);
	glDeleteShader(m_FragmentShader);
	glDeleteShader(m_VertexShader);
}

void Material::Initialize()
{
	//Load Shader
	ShaderLoader* sL = new ShaderLoader();
	if (m_UseGS)
	{
		m_ShaderProgram = sL->CreateGSProgram(m_VSfile, m_GSfile, m_FSfile,
			 m_VertexShader, m_GeometryShader, m_FragmentShader);
	}
	else
	{
		m_ShaderProgram = sL->CreateShaderProgram(m_VSfile,
			m_FSfile, m_VertexShader, m_FragmentShader);
	}
	delete sL;

	//Jup (maybe temporary with texture manager)
	LoadTextures();

	//Get Access to uniforms
	m_UniMatModel = glGetUniformLocation(m_ShaderProgram, "model");
	m_UniMatWVP = glGetUniformLocation(m_ShaderProgram, "worldViewProj");
	AccessShaderAttributes();
}

void Material::UploadVariables(glm::mat4 matModel)
{
	glUseProgram(m_ShaderProgram);
	//Upload matrices
	glUniformMatrix4fv(m_UniMatModel, 1, GL_FALSE, glm::value_ptr(matModel));
	glUniformMatrix4fv(m_UniMatWVP, 1, GL_FALSE, glm::value_ptr(CAMERA->GetViewProj()));

	UploadDerivedVariables();
}
