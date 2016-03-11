#include "CubeMaterial.hpp"

#include "../../Content/TextureLoader.hpp"
#include "../../Base\Context.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

//Working singleton Set
#define TIME Context::GetInstance()->pTime

CubeMaterial::CubeMaterial(std::string texKpath, std::string texPpath)
	:Material("Resources/sceneVertexShader.glsl", "Resources/sceneFragmentShader.glsl", "", false)
	,m_TexKittenPath(texKpath)
	,m_TexPuppyPath(texPpath)
{
}
CubeMaterial::~CubeMaterial()
{
	glDeleteTextures(1, &m_TexKitten);
	glDeleteTextures(1, &m_TexPuppy);
}

void CubeMaterial::LoadTextures()
{
	//Load Textures
	TextureLoader* tL = new TextureLoader();
	m_TexKitten = tL->LoadImageToTexture("Resources/sample.png");
	m_TexPuppy = tL->LoadImageToTexture("Resources/sample2.png");
	delete tL;
	//Bind Textures to shader
	glUseProgram(m_ShaderProgram);
	glUniform1i(glGetUniformLocation(m_ShaderProgram, "texKitten"), 0);
	glUniform1i(glGetUniformLocation(m_ShaderProgram, "texPuppy"), 1);
}

void CubeMaterial::AccessShaderAttributes()
{
	m_UniTime = glGetUniformLocation(m_ShaderProgram, "time");
	m_UniColor = glGetUniformLocation(m_ShaderProgram, "overrideColor");
}

void CubeMaterial::UploadDerivedVariables()
{
	//Bind active textures
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_TexKitten);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, m_TexPuppy);
	//Upload time
	glUniform1f(m_UniTime, TIME->DeltaTime());
}

void CubeMaterial::SpecifyInputLayout()
{
	GLint posAttrib = glGetAttribLocation(m_ShaderProgram, "position");
	glEnableVertexAttribArray(posAttrib);
	glVertexAttribPointer(posAttrib, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), 0);

	GLint colAttrib = glGetAttribLocation(m_ShaderProgram, "color");
	glEnableVertexAttribArray(colAttrib);
	glVertexAttribPointer(colAttrib, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (void*)(3 * sizeof(GLfloat)));

	GLint texAttrib = glGetAttribLocation(m_ShaderProgram, "texcoord");
	glEnableVertexAttribArray(texAttrib);
	glVertexAttribPointer(texAttrib, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (void*)(6 * sizeof(GLfloat)));
}

void CubeMaterial::UpdateReflectionAtt(glm::mat4 model, glm::vec3 col)
{
	glUniformMatrix4fv(m_UniMatModel, 1, GL_FALSE, glm::value_ptr(model));
	glUniform3f(m_UniColor, col.x, col.y, col.z);
}