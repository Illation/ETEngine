#include "stdafx.hpp"
#include "SpriteComponent.hpp"

#include "SpriteRenderer.hpp"

SpriteComponent::SpriteComponent( const std::string& spriteAsset, vec2 pivot, vec4 color ) :
	m_SpriteAsset( spriteAsset ),
	m_Pivot( pivot ),
	m_Color( color ),
	m_pTexture( nullptr )
{

}

SpriteComponent::~SpriteComponent()
{}

void SpriteComponent::Initialize()
{
	m_pTexture = ContentManager::Load<TextureData>( m_SpriteAsset );
}

void SpriteComponent::SetTexture( const std::string& spriteAsset )
{
	m_SpriteAsset = spriteAsset;
	m_pTexture = ContentManager::Load<TextureData>( m_SpriteAsset );
}

void SpriteComponent::Draw()
{
	if(!m_pTexture)
		return;

	vec3 pos = TRANSFORM->GetPosition();
	vec3 scale = TRANSFORM->GetScale();
	SpriteRenderer::GetInstance()->Draw( m_pTexture, vec2( TRANSFORM->GetPosition().xy ),
										 m_Color, m_Pivot, vec2( TRANSFORM->GetScale().xy ),
										 TRANSFORM->GetRotation().Roll(), pos.z );
}