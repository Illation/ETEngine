#pragma once
#include "UIUtil.hpp"

enum class UIComponentType : uint8
{
	INVALID,
	SPRITE,
	FONT
};

class UIComponent
{
public:
	virtual UIComponentType GetType() { return UIComponentType::INVALID; }

	virtual void* GetResource() = 0;

	virtual vec4 GetColor() { return m_Color; }

protected:
	friend class UIContainer;

	vec4 m_Color = vec4( 1 );

private:
};

struct UISprite : UIComponent
{
public:
	virtual UIComponentType GetType() { return UIComponentType::SPRITE; }

	virtual void* GetResource() { return m_Texture; }

private:
	TextureData* m_Texture = nullptr;
};