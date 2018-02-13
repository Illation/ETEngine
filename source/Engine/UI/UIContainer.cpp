#include "stdafx.hpp"
#include "UIContainer.h"

#include <limits>
#include "SpriteRenderer.hpp"
#include "TextRenderer.hpp"

UIDynamicBox::~UIDynamicBox()
{
	for (auto child : m_RelativeChildren)delete child;
	for (auto child : m_DynamicChildren)delete child;
}

iRect UIDynamicBox::CalculateDimensions( const ivec2 &worldPos )
{
	m_WorldPos = worldPos;
	iRect ret = m_Rect;
	ret.pos = ret.pos + m_WorldPos;

	ivec2 minCorner = ivec2(std::numeric_limits<int32>::max());
	ivec2 maxCorner = ivec2(std::numeric_limits<int32>::lowest());
	for(auto child : m_RelativeChildren)
	{
		iRect childDim = child->CalculateDimensions( ret.pos );
		minCorner.x = std::min( minCorner.x, childDim.pos.x );
		minCorner.y = std::min( minCorner.y, childDim.pos.y );
		maxCorner.x = std::max( maxCorner.x, childDim.pos.x + childDim.size.x );
		maxCorner.y = std::max( maxCorner.y, childDim.pos.y + childDim.size.y );
	}

	ivec2 posOffset = ret.pos;
	m_Rect.size = ivec2( 0 );
	for(auto child : m_DynamicChildren)
	{
		ivec2 childSize = child->CalculateDimensions( posOffset ).size;
		switch(m_Mode)
		{
		case Mode::HORIZONTAL:
			posOffset.x += childSize.x;
			m_Rect.size.y = std::max(m_Rect.size.y, childSize.y);
			break;
		case Mode::VERTICAL:
			posOffset.y += childSize.y;
			m_Rect.size.x = std::max(m_Rect.size.x, childSize.x);
			break;
		}
	}

	switch(m_Mode)
	{
	case Mode::HORIZONTAL:
		m_Rect.size.x = posOffset.x - worldPos.x;
		break;
	case Mode::VERTICAL:
		m_Rect.size.y = posOffset.y - worldPos.y;
		break;
	}

	//Combine relative and dynamic for final size calculation
	minCorner.x = std::min( minCorner.x, m_Rect.pos.x );
	minCorner.y = std::min( minCorner.y, m_Rect.pos.y );
	maxCorner.x = std::max( maxCorner.x, m_Rect.pos.x + m_Rect.size.x );
	maxCorner.y = std::max( maxCorner.y, m_Rect.pos.y + m_Rect.size.y );
	m_Rect.pos = minCorner;
	m_Rect.size = maxCorner - minCorner;
	ret.pos = m_Rect.pos + m_WorldPos;
	ret.size = m_Rect.size;

	return ret;
}

bool UIDynamicBox::Draw( uint16 level ) 
{
	if (level <= m_Level)
	{
		//if (etm::nearEqualsV(m_WorldPos, ivec2(0)))
			CalculateDimensions(m_WorldPos);
		m_Level = level;
		return true;//has children, gotta go deeper
	}
	m_Level = level;
	bool ret = false;
	for(auto child : m_RelativeChildren)
	{
		ret |= child->Draw( level );
	}
	for(auto child : m_DynamicChildren)
	{
		ret |= child->Draw( level );
	}
	return ret;
}

void UIDynamicBox::Update()
{
	for (auto child : m_DynamicChildren)child->Update();
	for (auto child : m_RelativeChildren)child->Update();
}

void UIDynamicBox::AddChild( UIContainer* child, Positioning positioning )
{
	switch(positioning)
	{
	case Positioning::DYNAMIC:
		m_DynamicChildren.push_back( child );
		break;
	case Positioning::UI_RELATIVE:
		m_RelativeChildren.push_back( child );
		break;
	}
}

bool UIPortal::Draw(uint16 level)
{
	//Limit dimensions
	vec2 size = vec2((float)m_Rect.size.x, (float)m_Rect.size.y);
	vec2 pos = vec2((float)m_Rect.pos.x, (float)m_Rect.pos.y) + vec2((float)m_WorldPos.x, (float)m_WorldPos.y);

	ivec2 prevPos, prevSize;
	STATE->GetViewport(prevPos, prevSize);
	STATE->SetViewport(m_Rect.pos + m_WorldPos, m_Rect.size);

	//Render Background
	SpriteRenderer::GetInstance()->Draw(nullptr, pos, m_Color, vec2(0), size, 0, 1, SpriteScalingMode::SCREEN);
	SpriteRenderer::GetInstance()->Draw();

	//Render subcomponents
	if (m_Child)
	{
		bool draw = true;
		while (draw)
		{
			draw = m_Child->Draw(level);
			level++;
			SpriteRenderer::GetInstance()->Draw();
			TextRenderer::GetInstance()->Draw();
		}
	}
	//Restore viewport size
	STATE->SetViewport(prevPos, prevSize);
	return false;
}

iRect UIFixedContainer::CalculateDimensions(const ivec2 &worldPos)
{
	m_WorldPos = worldPos;
	iRect ret = m_Rect;
	ret.pos = ret.pos + m_WorldPos;
	return ret;
}