#include "stdafx.hpp"
#include "UIComponent.hpp"
#include "SpriteRenderer.hpp"
#include "TextureData.hpp"
#include "TextRenderer.hpp"

UIComponent::UIComponent( ivec2 size, ivec2 localPos ) :UIFixedContainer()
{
	m_Rect.size = size; 
	m_Rect.pos = localPos;
}

UISprite::UISprite(TextureData* tex) 
	:m_Texture(tex), 
	UIComponent(tex->GetResolution(), ivec2(0))
{ }

bool UISprite::Draw( uint16 level ) 
{
	UNUSED(level);
	SpriteRenderer::GetInstance()->Draw( m_Texture, etm::vecCast<float>(m_WorldPos+m_Rect.pos), m_Color, vec2( 0 ),
		etm::vecCast<float>(m_Rect.size), 0, 1, SpriteScalingMode::PIXEL );

	return false;
}

UIText::UIText(std::string text, SpriteFont* pFont)
	:m_Font(pFont),
	m_Text(text),
	UIComponent(TextRenderer::GetInstance()->GetTextSize(text, pFont), ivec2(0))
{
}

bool UIText::Draw(uint16 level)
{
	UNUSED(level);
	TextRenderer::GetInstance()->SetFont(m_Font);
	TextRenderer::GetInstance()->SetColor(m_Color);
	TextRenderer::GetInstance()->DrawText(m_Text, etm::vecCast<float>(m_WorldPos + m_Rect.pos));
	return false;
}
