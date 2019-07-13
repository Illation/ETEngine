#pragma once
#include "UIUtil.hpp"
#include "UIContainer.h"

class SpriteFont;

enum class UIComponentType : uint8
{
	INVALID,
	SPRITE,
	FONT
};

class UIComponent : public UIFixedContainer
{
public:
	UIComponent(ivec2 size, ivec2 localPos);

	virtual UIComponentType GetType() { return UIComponentType::INVALID; }

	virtual void* GetResource() = 0;
	virtual bool Draw( uint16 level ) = 0;

	virtual vec4 GetColor() { return m_Color; }

protected:
	friend class UIContainer;

	vec4 m_Color = vec4( 1 );
};

struct UISprite : public UIComponent
{
public:
	UISprite(TextureData* tex);

	virtual UIComponentType GetType() { return UIComponentType::SPRITE; }

	virtual void* GetResource() { return m_Texture; }

	bool Draw( uint16 level ) override;
private:
	TextureData* m_Texture = nullptr;
};

struct UIText : public UIComponent
{
public:
	UIText(std::string text, SpriteFont* pFont);

	virtual UIComponentType GetType() { return UIComponentType::FONT; }

	virtual void* GetResource() { return m_Font; }

	bool Draw( uint16 level ) override;
private:
	SpriteFont* m_Font = nullptr;
	std::string m_Text;
};