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

	//Drawing - return true if it has children, level indicates the depth 
	//breadth first in most cases, unless we are drawing a frame
	virtual bool Draw( uint16 level ) = 0;

protected:
	iRect m_Rect;
	ivec2 m_WorldPos;

	uint16 m_Level = std::numeric_limits<uint16>::max();
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
		UI_RELATIVE	
	};

	UIDynamicBox( UIDynamicBox::Mode mode ) :m_Mode( mode ) {}

	iRect CalculateDimensions( const ivec2 &worldPos);
	virtual bool Draw( uint16 level );

	void AddChild( UIContainer* child, Positioning positioning );
private:
	UIDynamicBox::Mode m_Mode;

	std::vector<UIContainer*> m_RelativeChildren;
	std::vector<UIContainer*> m_DynamicChildren;
};

class UIPortal : public UIContainer
{
	iRect CalculateDimensions(const ivec2 &worldPos);
	virtual bool Draw(uint16 level);

	void SetChild(UIContainer* child) { m_Child = child; }
	void SetColor(vec4 val) { m_Color = val; }

private:
	UIContainer* m_Child = nullptr;

	vec4 m_Color = vec4(1);
};