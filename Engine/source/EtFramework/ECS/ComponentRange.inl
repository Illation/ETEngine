#pragma once


namespace et {
namespace fw {


//==========================
// Component Range Iterator
//==========================


//---------------------------------
// ComponentRange::iterator::c-tor
//
// Construct a new view of the range and use it to iterate
//
template<typename TViewType>
ComponentRange<TViewType>::iterator::iterator(BaseComponentRange* const range)
	: m_View(new TViewType())
{
	m_View->Init(range);
}

//---------------------------------
// ComponentRange::iterator:: ++
//
// prefix - postfix is omitted as we do not wish to make a copy of the view type
//
template<typename TViewType>
typename ComponentRange<TViewType>::iterator& ComponentRange<TViewType>::iterator::operator++()
{
	m_View->Next(); // update all pointers in the view
	return *this;
}

//---------------------------------
// ComponentRange::iterator::IsEnd
//
template<typename TViewType>
bool ComponentRange<TViewType>::iterator::IsEnd() const
{
	return ((m_View == nullptr) || m_View->IsEnd());
}

//---------------------------------
// ComponentRange::iterator:: ==
//
// #note: doesn't return true for other iterator objects pointing to the same entity
//
template<typename TViewType>
bool ComponentRange<TViewType>::iterator::operator==(iterator const& other) const
{
	return (IsEnd() ? other.IsEnd() : (m_View == other.m_View));
}

//---------------------------------
// ComponentRange::iterator:: !=
//
template<typename TViewType>
bool ComponentRange<TViewType>::iterator::operator!=(iterator const& other) const
{
	return !(*this == other);
}


//=================
// Component Range
//=================


//-----------------------
// ComponentRange::c-tor
//
template<typename TViewType>
ComponentRange<TViewType>::ComponentRange(EcsController* const controller, Archetype* const archetype, size_t const offset, size_t const count)
	: BaseComponentRange(controller, archetype, offset, count)
{ }


} // namespace fw
} // namespace et
