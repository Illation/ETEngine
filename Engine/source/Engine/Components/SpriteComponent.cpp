#include "stdafx.h"
#include "SpriteComponent.h"

#include <EtCore/Content/ResourceManager.h>

#include <Engine/GraphicsHelper/SpriteRenderer.h>
#include <Engine/Graphics/TextureData.h>


SpriteComponent::SpriteComponent(T_Hash const spriteAsset, vec2 const& pivot, vec4 const& color) 
	: m_SpriteAsset(spriteAsset)
	, m_Pivot(pivot)
	, m_Color(color)
{}

void SpriteComponent::Initialize()
{
	m_Texture = ResourceManager::Instance()->GetAssetData<TextureData>(m_SpriteAsset);
}

void SpriteComponent::SetTexture(T_Hash const spriteAsset)
{
	m_SpriteAsset = spriteAsset;
	m_Texture = ResourceManager::Instance()->GetAssetData<TextureData>(m_SpriteAsset);
}

void SpriteComponent::Draw()
{
	if (m_Texture == nullptr)
	{
		return;
	}

	SpriteRenderer::GetInstance()->Draw(m_Texture.get(), 
		TRANSFORM->GetPosition().xy, 
		m_Color, 
		m_Pivot, 
		TRANSFORM->GetScale().xy, 
		TRANSFORM->GetRotation().Roll(), 
		TRANSFORM->GetPosition().z, 
		SpriteRenderer::E_ScalingMode::Texture);
}