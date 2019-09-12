#pragma once
#include "UIContainer.h"

class UISplitter : public UIFixedContainer
{
public:
	enum class Mode : uint8
	{
		HORIZONTAL,
		VERTICAL
	}; 

	UISplitter(UISplitter::Mode mode) :UIFixedContainer(), m_Mode(mode) {}
	virtual ~UISplitter();
	virtual bool Draw(uint16 level);

	void Update() override;

	void SetSize(ivec2 size)override;
	void SetSizeOnly(ivec2 size)override;
	void SetSplitPercentage(float perc);

	void SetFirst(UIFixedContainer* child) { m_First = child; }
	void SetSecond(UIFixedContainer* child) { m_Second = child; }
private:
	void RecalculateSplit(bool sizeOnly = false);

	UIFixedContainer* m_First = nullptr;
	UIFixedContainer* m_Second = nullptr;

	float m_SplitRegionPix = 5;
	bool m_DragActive = false;
	bool m_IsHovering = false;

	float m_SplitPercentage = 0.2f;
	UISplitter::Mode m_Mode;
};

