#include "stdafx.h"
#include "UIComponent.h"

#include <Engine/GraphicsHelper/SpriteRenderer.h>
#include <Engine/GraphicsHelper/TextRenderer.h>


UIComponent::UIComponent( ivec2 size, ivec2 localPos ) 
	: UIFixedContainer()
{
	m_Rect.size = size; 
	m_Rect.pos = localPos;
}

UISprite::UISprite(AssetPtr<TextureData> tex)
	: UIComponent(tex->GetResolution(), ivec2(0))
	, m_Texture(tex)
{ }

bool UISprite::Draw( uint16 level ) 
{
	UNUSED(level);
	SpriteRenderer::GetInstance()->Draw(m_Texture.get(), etm::vecCast<float>(m_WorldPos+m_Rect.pos), m_Color, vec2( 0 ),
		etm::vecCast<float>(m_Rect.size), 0, 1, SpriteRenderer::E_ScalingMode::Pixel );

	return false;
}

UIText::UIText(std::string text, AssetPtr<SpriteFont> font)
	: UIComponent(TextRenderer::GetInstance()->GetTextSize(text, font.get()), ivec2(0))
	, m_Font(font)
	, m_Text(text)
{ }

bool UIText::Draw(uint16 level)
{
	UNUSED(level);
	TextRenderer::GetInstance()->SetFont(m_Font.get());
	TextRenderer::GetInstance()->SetColor(m_Color);
	TextRenderer::GetInstance()->DrawText(m_Text, etm::vecCast<float>(m_WorldPos + m_Rect.pos));
	return false;
}
