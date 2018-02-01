#include "stdafx.hpp"
#include "UIComponent.hpp"
#include "SpriteRenderer.hpp"

UIComponent::UIComponent( ivec2 size, ivec2 localPos ) 
{
	m_Rect.size = size; 
	m_Rect.pos = localPos;
}

iRect UIComponent::CalculateDimensions( const ivec2 &worldPos )
{
	m_WorldPos = worldPos;
	iRect ret = m_Rect;
	ret.pos = ret.pos + m_WorldPos;
	return ret;
}

bool UISprite::Draw( uint16 level ) 
{
	UNUSED(level);
	SpriteRenderer::GetInstance()->Draw( m_Texture, etm::vecCast<float>(m_WorldPos), m_Color, vec2( 0 ), 
		etm::vecCast<float>(m_Rect.size), 0, 1, SpriteScalingMode::TEXTURE );

	return false;
}
