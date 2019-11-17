#include "stdafx.h"
#include "SpriteComponent.h"

#include <Engine/SceneGraph/SceneManager.h>


SpriteComponent::SpriteComponent(T_Hash const textureAsset, vec2 const& pivot, vec4 const& color)
	: m_TextureAssetId(textureAsset)
	, m_Pivot(pivot)
	, m_Color(color)
{}

SpriteComponent::~SpriteComponent()
{
	if (m_Id != core::INVALID_SLOT_ID)
	{
		SceneManager::GetInstance()->GetRenderScene().RemoveSprite(m_Id);
	}
}

void SpriteComponent::Initialize()
{
	m_Id = SceneManager::GetInstance()->GetRenderScene().AddSprite(m_TextureAssetId, GetTransform()->GetNodeId(), m_Pivot, m_Color);
}

void SpriteComponent::SetTexture(T_Hash const textureAsset)
{
	m_TextureAssetId = textureAsset;
	if (m_Id != core::INVALID_SLOT_ID)
	{
		SceneManager::GetInstance()->GetRenderScene().UpdateSpriteTexture(m_Id, m_TextureAssetId);
	}
}

void SpriteComponent::SetPivot(vec2 const& pivot)
{
	m_Pivot = pivot;
	if (m_Id != core::INVALID_SLOT_ID)
	{
		SceneManager::GetInstance()->GetRenderScene().UpdateSpritePivot(m_Id, m_Pivot);
	}
}

void SpriteComponent::SetColor(vec4 const& color)
{
	m_Color = color;
	if (m_Id != core::INVALID_SLOT_ID)
	{
		SceneManager::GetInstance()->GetRenderScene().UpdateSpriteColor(m_Id, m_Color);
	}
}
