#include "stdafx.hpp"
#include "UIContainer.h"

#include <limits>

UIContentBox::UIContentBox( ivec2 size, ivec2 localPos, UIComponent* comp ) 
	:m_Content( comp )
{
	m_Rect.size = size; 
	m_Rect.pos = localPos;
}

iRect UIContentBox::CalculateDimensions( const ivec2 &worldPos )
{
	m_WorldPos = worldPos;
	iRect ret = m_Rect;
	ret.pos = ret.pos + m_WorldPos;
	return ret;
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
		minCorner.x = min( minCorner.x, childDim.pos.x );
		minCorner.y = min( minCorner.y, childDim.pos.y );
		maxCorner.x = max( maxCorner.x, childDim.pos.x + childDim.size.x );
		maxCorner.y = max( maxCorner.y, childDim.pos.y + childDim.size.y );
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
			m_Rect.size.y = max(m_Rect.size.y, childSize.y);
			break;
		case Mode::VERTICAL:
			posOffset.y += childSize.y;
			m_Rect.size.x = max(m_Rect.size.x, childSize.x);
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
	return ret;
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
	case Positioning::SEPARATE:
		m_SeparateChildren.push_back( child );
		break;
	}
}
