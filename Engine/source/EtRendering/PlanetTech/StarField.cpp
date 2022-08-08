#include "stdafx.h"
#include "StarField.h"

#include <EtCore/Content/ResourceManager.h>
#include <EtCore/Content/AssetStub.h>
#include <EtCore/IO/JsonParser.h>

#include <EtRHI/GraphicsTypes/Shader.h>
#include <EtRHI/GraphicsTypes/TextureData.h>


namespace et {
namespace render {


StarField::StarField(core::HashString const assetId)
{
	AssetPtr<core::StubData> jsonDbText = core::ResourceManager::Instance()->GetAssetData<core::StubData>(assetId);

	core::JSON::Parser parser = core::JSON::Parser(std::string(jsonDbText->GetText(), jsonDbText->GetLength()));
	core::JSON::Object* root = parser.GetRoot();

	uint32 starCount = 0;

	core::JSON::Array* jstarArray = (*root)["stars"]->arr();
	for (auto jStar : jstarArray->value)
	{
		vec4 star;
		if (core::JSON::ArrayVector(jStar, star) && m_MaxStars == 0 ? true : starCount < m_MaxStars)
		{
			//HYG coordinates are in a different coordinate system Z up Y forward 
			//first component is magnitude
			//output [X, Z, Y, mag]
			m_Stars.push_back(vec4(star[1], star[3], star[2], star[0]));
			starCount++;
		}
	}

	rhi::I_GraphicsContextApi* const api = rhi::ContextHolder::GetRenderContext();

	m_pShader = core::ResourceManager::Instance()->GetAssetData<rhi::ShaderData>(core::HashString("Shaders/FwdStarField.glsl"));
	m_pSprite = core::ResourceManager::Instance()->GetAssetData<rhi::TextureData>(core::HashString("Textures/starSprite.png"));

	//Generate buffers and arrays
	m_VAO = api->CreateVertexArray();
	m_VBO = api->CreateBuffer();

	//bind
	api->BindVertexArray(m_VAO);
	api->BindBuffer(rhi::E_BufferType::Vertex, m_VBO);

	//set data and attributes
	api->SetBufferData(rhi::E_BufferType::Vertex, m_Stars.size() * sizeof(vec4), m_Stars.data(), rhi::E_UsageHint::Dynamic);

	api->SetVertexAttributeArrayEnabled(0, true);
	api->DefineVertexAttributePointer(0, 4, rhi::E_DataType::Float, false, sizeof(vec4), 0);

	//unbind
	api->BindBuffer(rhi::E_BufferType::Vertex, 0);
	api->BindVertexArray(0);
}

StarField::~StarField()
{
	rhi::I_GraphicsContextApi* const api = rhi::ContextHolder::GetRenderContext();

	api->DeleteVertexArray(m_VAO);
	api->DeleteBuffer(m_VBO);
}

void StarField::Draw(Camera const& cam) const
{
	rhi::I_GraphicsContextApi* const api = rhi::ContextHolder::GetRenderContext();

	api->SetBlendEnabled(true);
	api->SetBlendEquation(rhi::E_BlendEquation::Add);
	api->SetBlendFunction(rhi::E_BlendFactor::One, rhi::E_BlendFactor::Zero);

	api->BindVertexArray(m_VAO);
	api->SetShader(m_pShader.get());
	m_pShader->Upload("uTexture"_hash, m_pSprite.get());
	m_pShader->Upload("uRadius"_hash, m_Radius);
	m_pShader->Upload("uBaseFlux"_hash, m_BaseFlux);
	m_pShader->Upload("uBaseMag"_hash, m_BaseMag);
	m_pShader->Upload("uAspectRatio"_hash, rhi::Viewport::GetCurrentViewport()->GetAspectRatio());
	api->DrawArrays(rhi::E_DrawMode::Points, 0, m_DrawnStars);
	api->BindVertexArray(0);
	api->SetBlendEnabled(false);
}


} // namespace render
} // namespace et
