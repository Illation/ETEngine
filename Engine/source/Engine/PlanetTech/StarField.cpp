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
	GraphicsApiContext* const api = Viewport::GetCurrentApiContext();

	api->DeleteVertexArrays(1, &m_VAO);
	api->DeleteBuffers(1, &m_VBO);
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

	GraphicsApiContext* const api = Viewport::GetCurrentApiContext();

	m_pShader = ResourceManager::Instance()->GetAssetData<ShaderData>("FwdStarField.glsl"_hash);
	m_pSprite = ResourceManager::Instance()->GetAssetData<TextureData>("starSprite.png"_hash);

	api->SetShader(m_pShader.get());
	m_pShader->Upload("uTexture"_hash, 0);

	//Generate buffers and arrays
	api->GenerateVertexArrays(1, &m_VAO);
	api->GenerateBuffers(1, &m_VBO);

	//bind
	api->BindVertexArray(m_VAO);
	api->BindBuffer(GL_ARRAY_BUFFER, m_VBO);

	//set data and attributes
	api->SetBufferData(GL_ARRAY_BUFFER, m_Stars.size()*sizeof(vec4), m_Stars.data(), GL_DYNAMIC_DRAW);

	api->SetVertexAttributeArrayEnabled(0, true);
	api->DefineVertexAttributePointer(0, (GLint)4, GL_FLOAT, GL_FALSE, (GLsizei)sizeof(vec4), (GLvoid*)0);

	//unbind
	api->BindBuffer(GL_ARRAY_BUFFER, 0);
	api->BindVertexArray(0);
}

void StarField::DrawForward()
{
	GraphicsApiContext* const api = Viewport::GetCurrentApiContext();

	api->SetBlendEnabled(true);
	api->SetBlendEquation(GL_FUNC_ADD);
	api->SetBlendFunction(GL_ONE, GL_ZERO);

	api->BindVertexArray(m_VAO);
	api->SetShader(m_pShader.get());
	api->SetActiveTexture(0);
	api->BindTexture(m_pSprite->GetTarget(), m_pSprite->GetHandle());
	m_pShader->Upload("viewProj"_hash, CAMERA->GetStatViewProj());
	m_pShader->Upload("viewInv"_hash, CAMERA->GetViewInv());
	m_pShader->Upload("uRadius"_hash, m_Radius);
	m_pShader->Upload("uBaseFlux"_hash, m_BaseFlux);
	m_pShader->Upload("uBaseMag"_hash, m_BaseMag);
	m_pShader->Upload("uAspectRatio"_hash, Config::GetInstance()->GetWindow().AspectRatio);
	api->DrawArrays(GL_POINTS, 0, m_DrawnStars);
	api->BindVertexArray(0);
	api->SetBlendEnabled(false);
}
