#include "stdafx.h"
#include "StarField.h"

#include <glad/glad.h>

#include <EtCore/Content/ResourceManager.h>
#include <EtCore/Content/AssetStub.h>
#include <EtCore/FileSystem/Json/JsonParser.h>

#include <Engine/Graphics/Shader.h>
#include <Engine/Graphics/TextureData.h>


StarField::StarField(T_Hash const assetId) 
	: m_AssetId(assetId)
{ }

StarField::~StarField()
{
	glDeleteVertexArrays(1, &m_VAO);
	glDeleteBuffers(1, &m_VBO);
}

void StarField::Initialize()
{
	AssetPtr<StubData> jsonDbText = ResourceManager::Instance()->GetAssetData<StubData>(m_AssetId);

	JSON::Parser parser = JSON::Parser(std::string(jsonDbText->GetText(), jsonDbText->GetLength()));
	JSON::Object* root = parser.GetRoot();

	uint32 starCount = 0;

	JSON::Array* jstarArray = (*root)["stars"]->arr();
	for (auto jStar : jstarArray->value)
	{
		vec4 star;
		if (JSON::ArrayVector(jStar, star) && m_MaxStars == 0 ? true : starCount < m_MaxStars)
		{
			//HYG coordinates are in a different coordinate system Z up Y forward 
			//first component is magnitude
			//output [X, Z, Y, mag]
			m_Stars.push_back(vec4(star[1], star[3], star[2], star[0]));
			starCount++;
		}
	}
	
	m_pShader = ResourceManager::Instance()->GetAssetData<ShaderData>("FwdStarField.glsl"_hash);
	m_pSprite = ResourceManager::Instance()->GetAssetData<TextureData>("starSprite.png"_hash);

	STATE->SetShader(m_pShader.get());
	m_pShader->Upload("uTexture"_hash, 0);

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
	STATE->SetShader(m_pShader.get());
	STATE->SetActiveTexture(0);
	STATE->BindTexture(m_pSprite->GetTarget(), m_pSprite->GetHandle());
	m_pShader->Upload("viewProj"_hash, CAMERA->GetStatViewProj());
	m_pShader->Upload("viewInv"_hash, CAMERA->GetViewInv());
	m_pShader->Upload("uRadius"_hash, m_Radius);
	m_pShader->Upload("uBaseFlux"_hash, m_BaseFlux);
	m_pShader->Upload("uBaseMag"_hash, m_BaseMag);
	m_pShader->Upload("uAspectRatio"_hash, Config::GetInstance()->GetWindow().AspectRatio);
	STATE->DrawArrays(GL_POINTS, 0, m_DrawnStars);
	STATE->BindVertexArray(0);
	STATE->SetBlendEnabled(false);
}
