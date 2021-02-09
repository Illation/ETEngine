#include "stdafx.h"
#include "XmlDom.h"


namespace et {
namespace core {

namespace XML {


//-----------------------
// Element::GetAttribute
//
Attribute const* Element::GetAttribute(T_Hash const id) const
{
	auto const foundIt = std::find_if(m_Attributes.cbegin(), m_Attributes.cend(), [id](Attribute const& attribute)
		{
			return (id == attribute.m_Name.Get());
		});

	if (foundIt == m_Attributes.cend())
	{
		return nullptr;
	}

	return &(*foundIt);
}

//------------------------
// Element::GetFirstChild
//
// Return the first child element with matching ID non-recursively
//  - if no child is found nullptr is returned 
//
Element const* Element::GetFirstChild(T_Hash const id) const
{
	auto const foundIt = std::find_if(m_Children.cbegin(), m_Children.cend(), [id](Element const& attribute)
		{
			return (id == attribute.m_Name.Get());
		});

	if (foundIt == m_Children.cend())
	{
		return nullptr;
	}

	return &(*foundIt);
}

//------------------------
// Element::GetFirstChild
//
// Return the first child element with matching ID non-recursively
//  - pos can be used to offset search
//  - the index of the found child is written pos
//  - if no child is found nullptr is returned and pos is left unmodified
//
Element const* Element::GetFirstChild(T_Hash const id, size_t& pos) const
{
	auto const foundIt = std::find_if(m_Children.cbegin() + pos, m_Children.cend(), [id](Element const& attribute)
		{
			return (id == attribute.m_Name.Get());
		});

	if (foundIt == m_Children.cend())
	{
		return nullptr;
	}

	pos = static_cast<size_t>(foundIt - m_Children.cbegin());
	return &(*foundIt);
}


} // namespace XML

} // namespace core
} // namespace et
