#pragma once
#include "UIUtil.hpp"
#include "UIContainer.h"

enum class UIComponentType : uint8
{
	INVALID,
	SPRITE,
	FONT
};

class UIComponent : public UIContainer
{
public:
	UIComponent(ivec2 size, ivec2 localPos);

	virtual UIComponentType GetType() { return UIComponentType::INVALID; }

	virtual void* GetResource() = 0;
	virtual bool Draw( uint16 level ) = 0;

	virtual vec4 GetColor() { return m_Color; }

	iRect CalculateDimensions( const ivec2 &worldPos);

	void SetSize( ivec2 size ) { m_Rect.size = size; }

protected:
	friend class UIContainer;

	vec4 m_Color = vec4( 1 );
};

struct UISprite : UIComponent
{
public:
	virtual UIComponentType GetType() { return UIComponentType::SPRITE; }

	virtual void* GetResource() { return m_Texture; }

	bool Draw( uint16 level ) override;

private:
	TextureData* m_Texture = nullptr;
};