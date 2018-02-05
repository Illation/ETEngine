#include "stdafx.hpp"
#include "UIPadding.h"
#include "SpriteRenderer.hpp"

UIFixedPadding::UIFixedPadding(UIFixedContainer* child, ivec4 padding, vec4 color) 
	:UIFixedContainer(), m_Child(child), m_Padding(padding), m_Color(color)
{ }

UIFixedPadding::~UIFixedPadding()
{
	delete m_Child;
}

bool UIFixedPadding::Draw(uint16 level)
{
	//if alpha is 0 we don't bother to draw the padding sprite
	if (etm::nearEquals(m_Color.a, 0.f))return m_Child->Draw(level);

	if (level <= m_Level)
	{
		vec2 pos = etm::vecCast<float>(m_Rect.pos) + etm::vecCast<float>(m_WorldPos);

		SpriteRenderer::GetInstance()->Draw(nullptr, pos, m_Color, vec2(0), etm::vecCast<float>(m_Rect.size), 0, 1, SpriteScalingMode::SCREEN);

		m_Level = level;
		return true;//has children, gotta go deeper
	}
	m_Level = level;
	return m_Child->Draw(level);
}

void UIFixedPadding::SetSize(ivec2 size)
{
	m_Rect.size = size;
	m_Child->SetLocalPos(m_WorldPos + m_Rect.pos + m_Padding.xy);
	m_Child->SetSize(m_Rect.size - (m_Padding.xy + m_Padding.zw));
}

void UIFixedPadding::SetSizeOnly(ivec2 size)
{
	m_Rect.size = size;
	m_Child->SetLocalPos(m_WorldPos + m_Rect.pos + m_Padding.xy);
	m_Child->SetSizeOnly(m_Rect.size - (m_Padding.xy + m_Padding.zw));
}

UIDynamicPadding::UIDynamicPadding(UIContainer* child, ivec4 padding /*= ivec4(2)*/, vec4 color /*= vec4(0)*/)
	:UIContainer(), m_Child(child), m_Padding(padding), m_Color(color)
{ }

iRect UIDynamicPadding::CalculateDimensions(const ivec2 &worldPos)
{
	iRect childDim = m_Child->CalculateDimensions(worldPos);
	m_Rect.pos = childDim.pos - m_Padding.xy;
	m_Rect.size = childDim.size + m_Padding.xy + m_Padding.zw;
	m_WorldPos = ivec2(0);
	iRect ret = m_Rect;
	ret.pos = ret.pos + m_WorldPos;
	return ret;
}

bool UIDynamicPadding::Draw(uint16 level)
{
	if (level <= m_Level)
	{
		CalculateDimensions(m_WorldPos);

		if (!etm::nearEquals(m_Color.a, 0.f))
		{
			vec2 pos = etm::vecCast<float>(m_Rect.pos) + etm::vecCast<float>(m_WorldPos);
			SpriteRenderer::GetInstance()->Draw(nullptr, pos, m_Color, vec2(0), etm::vecCast<float>(m_Rect.size), 0, 1, SpriteScalingMode::SCREEN);
		}

		m_Level = level;
		return true;//has children, gotta go deeper
	}
	m_Level = level;
	return m_Child->Draw(level);
}
