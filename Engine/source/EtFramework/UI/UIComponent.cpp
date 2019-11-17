#include "stdafx.h"
#include "UIComponent.h"

#include <EtRendering/SceneRendering/ShadedSceneRenderer.h>


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
	render::ShadedSceneRenderer::GetCurrent()->GetSpriteRenderer().Draw(m_Texture.get(), etm::vecCast<float>(m_WorldPos+m_Rect.pos), m_Color, vec2( 0 ),
		etm::vecCast<float>(m_Rect.size), 0, 1, SpriteRenderer::E_ScalingMode::Pixel );

	return false;
}

UIText::UIText(std::string text, AssetPtr<SpriteFont> font)
	: UIComponent(render::ShadedSceneRenderer::GetCurrent()->GetTextRenderer().GetTextSize(text, font.get()), ivec2(0))
	, m_Font(font)
	, m_Text(text)
{ }

bool UIText::Draw(uint16 level)
{
	UNUSED(level);

	TextRenderer& textRenderer = render::ShadedSceneRenderer::GetCurrent()->GetTextRenderer();

	textRenderer.SetFont(m_Font.get());
	textRenderer.SetColor(m_Color);
	textRenderer.DrawText(m_Text, etm::vecCast<float>(m_WorldPos + m_Rect.pos));

	return false;
}
