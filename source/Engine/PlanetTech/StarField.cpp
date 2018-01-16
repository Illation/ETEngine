#include "stdafx.hpp"
#include "StarField.h"
#include "FileSystem\Entry.h"
#include "FileSystem\JSONparser.h"
#include "FileSystem\FileUtil.h"
#include "ShaderData.hpp"
#include "TextureData.hpp"

StarField::StarField(const std::string &dataFile) :
	m_DataFile(dataFile)
{ }

StarField::~StarField()
{
	glDeleteVertexArrays(1, &m_VAO);
	glDeleteBuffers(1, &m_VBO);
}

void StarField::Initialize()
{
	File* jsonFile = new File(m_DataFile, nullptr);
	if (!jsonFile->Open(FILE_ACCESS_MODE::Read))
		return;

	JSON::Parser parser = JSON::Parser(FileUtil::AsText(jsonFile->Read()));
	delete jsonFile;
	jsonFile = nullptr;

	JSON::Object* root = parser.GetRoot();
	uint32 starCount = 0;

	JSON::Array* jstarArray = (*root)["stars"]->arr();
	for (auto jStar : jstarArray->value)
	{
		vec4 star;
		if (JSON::ArrayVector(jStar, star) && m_MaxStars == 0 ? true : starCount < m_MaxStars)
		{
			m_Stars.push_back(star);
			starCount++;
		}
	}
	
	m_pShader = ContentManager::Load<ShaderData>("Shaders/FwdStarField.glsl");
	m_pSprite = ContentManager::Load<TextureData>("Resources/Textures/starSprite.png");

	STATE->SetShader(m_pShader);
	glUniform1i(glGetUniformLocation(m_pShader->GetProgram(), "uTexture"), 0);

	//Generate buffers and arrays
	glGenVertexArrays(1, &m_VAO);
	glGenBuffers(1, &m_VBO);

	//bind
	STATE->BindVertexArray(m_VAO);
	STATE->BindBuffer(GL_ARRAY_BUFFER, m_VBO);

	//set data and attributes
	glBufferData(GL_ARRAY_BUFFER, m_Stars.size()*sizeof(vec4), m_Stars.data(), GL_DYNAMIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, (GLint)4, GL_FLOAT, GL_FALSE, (GLsizei)sizeof(vec4), (GLvoid*)0);

	//unbind
	STATE->BindBuffer(GL_ARRAY_BUFFER, 0);
	STATE->BindVertexArray(0);
}

void StarField::DrawForward()
{
	STATE->SetBlendEnabled(true);
	STATE->SetBlendEquation(GL_FUNC_ADD);
	STATE->SetBlendFunction(GL_ONE, GL_ZERO);

	STATE->BindVertexArray(m_VAO);
	STATE->SetShader(m_pShader);
	STATE->SetActiveTexture(0);
	STATE->BindTexture(m_pSprite->GetTarget(), m_pSprite->GetHandle());
	glUniformMatrix4fv(glGetUniformLocation(m_pShader->GetProgram(), "viewProj"), 1, GL_FALSE, etm::valuePtr(CAMERA->GetStatViewProj()));
	glUniform1f(glGetUniformLocation(m_pShader->GetProgram(), "uRadius"), m_Radius);
	glUniform1f(glGetUniformLocation(m_pShader->GetProgram(), "uBrightnessMult"), m_BrightnessMult);
	glDrawArrays(GL_POINTS, 0, m_DrawnStars);
	PERFORMANCE->m_DrawCalls++;
	STATE->BindVertexArray(0);
	STATE->SetBlendEnabled(false);
}
