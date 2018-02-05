#pragma once
#include "UIContainer.h"

class UIFixedPadding : public UIFixedContainer
{
public:
	UIFixedPadding(UIFixedContainer* child, ivec4 padding = ivec4(2), vec4 color = vec4(0));
	virtual ~UIFixedPadding();

	void Update() override { m_Child->Update(); }

	bool Draw(uint16 level) override;

	void SetSize(ivec2 size)override;
	void SetSizeOnly(ivec2 size)override;
private:
	UIFixedContainer* m_Child = nullptr;
	ivec4 m_Padding = ivec4(2);//left top right bottom
	vec4 m_Color = vec4(0);
};

class UIDynamicPadding : public UIContainer
{
public:
	UIDynamicPadding(UIContainer* child, ivec4 padding = ivec4(2), vec4 color = vec4(0));
	virtual ~UIDynamicPadding() { delete m_Child; }

	iRect CalculateDimensions(const ivec2 &worldPos) override;

	void Update() override { m_Child->Update(); }

	bool Draw(uint16 level) override;
private:
	UIContainer* m_Child = nullptr;
	ivec4 m_Padding;
	vec4 m_Color;
};