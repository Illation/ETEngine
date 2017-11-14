#pragma once
#include "UIUtil.hpp"

class UIComponent;

class UIContainer
{
public:
	virtual iRect CalculateDimensions() = 0;

protected:
	iRect m_Rect;
};

class UIContentBox : public UIContainer
{
	UIContentBox(ivec2 size, ivec2 localPos, UIComponent* comp)
		:m_Rect.size(size), m_Rect.pos(localPos), m_Content(comp)
	{ }

	iRect CalculateDimensions() { return m_Rect; }

	void SetSize( ivec2 size ) { m_Rect.size = size; }
	void SetLocalPos( ivec2 pos ) { m_Rect.pos = pos; }
private:
	UIComponent* m_Content;
};