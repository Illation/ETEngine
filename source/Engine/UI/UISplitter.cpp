#include "stdafx.hpp"
#include "UISplitter.h"


UISplitter::~UISplitter()
{
	delete m_First;
	delete m_Second;
}

bool UISplitter::Draw(uint16 level)
{
	bool ret = false;
	ret |= m_First->Draw(level);
	ret |= m_Second->Draw(level);
	return ret;
}

void UISplitter::Update()
{
	vec2 mousePos = INPUT->GetMousePosition();
	iRect overlapRegion;
	switch (m_Mode)
	{
	case UISplitter::Mode::HORIZONTAL:
		overlapRegion = iRect(
			ivec2((int32)((m_Rect.size.x*m_SplitPercentage) - m_SplitRegionPix), 0),
			ivec2((int32)m_SplitRegionPix * 2, m_Rect.size.y));
		break;
	case UISplitter::Mode::VERTICAL:
		overlapRegion = iRect(
			ivec2(0, (int32)((m_Rect.size.y*m_SplitPercentage) - m_SplitRegionPix)),
			ivec2(m_Rect.size.x, (int32)m_SplitRegionPix * 2));
		break;
	}
	if (overlapRegion.Contains(mousePos))
	{
		if (INPUT->IsMouseButtonPressed(SDL_BUTTON_LEFT))m_DragActive = true;
		if (!m_IsHovering)
		{
			m_IsHovering = true;
			INPUT->SetSystemCursor(m_Mode == UISplitter::Mode::HORIZONTAL ? SDL_SYSTEM_CURSOR_SIZEWE : SDL_SYSTEM_CURSOR_SIZENS);
		}
	}
	else if (m_IsHovering)
	{
		m_IsHovering = false;
		INPUT->SetSystemCursor(SDL_SYSTEM_CURSOR_ARROW);
	}
	if (m_DragActive)
	{
		switch (m_Mode)
		{
		case UISplitter::Mode::HORIZONTAL:
			SetSplitPercentage((mousePos.x - (float)m_Rect.pos.x) / (float)m_Rect.size.x);
			break;
		case UISplitter::Mode::VERTICAL:
			SetSplitPercentage((mousePos.y - (float)m_Rect.pos.y) / (float)m_Rect.size.y);
			break;
		}
		if (INPUT->IsMouseButtonReleased(SDL_BUTTON_LEFT))
		{
			m_DragActive = false;
		}
		INPUT->ConsumeMouse();
	}
	m_First->Update();
	m_Second->Update();
}

void UISplitter::SetSize(ivec2 size)
{
	m_Rect.size = size;
	RecalculateSplit();
}

void UISplitter::SetSizeOnly(ivec2 size)
{
	m_Rect.size = size;
	RecalculateSplit(true);
}

void UISplitter::SetSplitPercentage(float perc)
{
	m_SplitPercentage = perc;
	RecalculateSplit(true);
}

void UISplitter::RecalculateSplit(bool sizeOnly)
{
	ivec2 firstSize;
	ivec2 secondSize;
	switch (m_Mode)
	{
	case UISplitter::Mode::HORIZONTAL:
		firstSize = ivec2((int32)(m_Rect.size.x*m_SplitPercentage), m_Rect.size.y);
		m_First->SetLocalPos(ivec2(0));
		secondSize = ivec2((int32)(m_Rect.size.x*(1 - m_SplitPercentage)), m_Rect.size.y);
		m_Second->SetLocalPos(ivec2((int32)(m_Rect.size.x*m_SplitPercentage), 0));
		break;
	case UISplitter::Mode::VERTICAL:
		firstSize = ivec2(m_Rect.size.x, (int32)(m_Rect.size.y*m_SplitPercentage));
		m_First->SetLocalPos(ivec2(0));
		secondSize = ivec2(m_Rect.size.x, (int32)(m_Rect.size.y*(1 - m_SplitPercentage)));
		m_Second->SetLocalPos(ivec2(0, (int32)(m_Rect.size.y*m_SplitPercentage)));
		break;
	}
	if (sizeOnly)
	{
		m_First->SetSizeOnly(firstSize);
		m_Second->SetSizeOnly(secondSize);
	}
	else
	{
		m_First->SetSize(firstSize);
		m_Second->SetSize(secondSize);
	}
}