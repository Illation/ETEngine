#pragma once
#include "UIUtil.hpp"

class UIComponent;

class UIContainer
{
public:
	//Should set the world position and return a rectangle in world position and its local scale
	virtual iRect CalculateDimensions(const ivec2 &worldPos) = 0;

	//offset to world position
	void SetLocalPos( ivec2 pos ) { m_Rect.pos = pos; }

protected:
	iRect m_Rect;
	ivec2 m_WorldPos;
};

class UIContentBox : public UIContainer
{
public:
	UIContentBox(ivec2 size, ivec2 localPos, UIComponent* comp);

	iRect CalculateDimensions( const ivec2 &worldPos);

	void SetSize( ivec2 size ) { m_Rect.size = size; }

	UIComponent* GetContent() { return m_Content; }
private:
	UIComponent* m_Content;
};

class UIDynamicBox : public UIContainer
{
public:
	enum class Mode : uint8
	{
		HORIZONTAL,
		VERTICAL
	};

	enum class Positioning : uint8
	{
		DYNAMIC,	
		UI_RELATIVE,	
		SEPARATE
	};

	UIDynamicBox( UIDynamicBox::Mode mode ) :m_Mode( mode ) {}

	iRect CalculateDimensions( const ivec2 &worldPos);
	void AddChild( UIContainer* child, Positioning positioning );
private:
	UIDynamicBox::Mode m_Mode;

	std::vector<UIContainer*> m_RelativeChildren;
	std::vector<UIContainer*> m_DynamicChildren;
	std::vector<UIContainer*> m_SeparateChildren;
};