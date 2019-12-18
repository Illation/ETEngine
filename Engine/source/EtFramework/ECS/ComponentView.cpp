#include "stdafx.h"
#include "Archetype.h"
#include "ComponentView.h"


namespace framework {


//====================
// Component Accessor
//====================


//-------------------------
// ComponentView::Accessor
//
// Iteratable view of a selection of components
//
ComponentView::Accessor::Accessor(void** pointerRef, T_CompTypeIdx const type, bool const readAccess) 
	: currentElement(reinterpret_cast<uint8*&>(*pointerRef))
	, typeIdx(type)
	, read(readAccess)
{ } 


//================
// Component View
//================


//----------------------
// ComponentView::Init
//
// initialize references to all pointers
//
void ComponentView::Init(BaseComponentRange* const range)
{
	m_Range = range;

	Register();

	// set the pointers to the position within the pool that the range starts from
	for (Accessor& access : m_Accessors)
	{
		access.currentElement = static_cast<uint8*>(m_Range->m_Archetype->GetPool(access.typeIdx).At(m_Range->m_Offset));
	}
}

//----------------------
// ComponentView::IsEnd
//
// Whether we are upon the last entity in the range
//
bool ComponentView::IsEnd() const
{
	return (m_Current >= m_Range->m_Count);
}

//----------------------------
// ComponentView::GetTypeList
//
// List of component types we access
//
T_CompTypeList ComponentView::GetTypeList() const
{
	T_CompTypeList ret;
	for (Accessor const& access : m_Accessors)
	{
		ret.emplace_back(access.typeIdx);
	}

	return ret;
}

//---------------------
// ComponentView::Next
//
// Increment component pointers
//
bool ComponentView::Next()
{
	m_Current++;
	if (m_Current >= m_Range->m_Count)
	{
		return true;
	}

	for (Accessor& access : m_Accessors)
	{
		access.currentElement += ComponentRegistry::Instance().GetSize(access.typeIdx);
	}

	return false;
}


} // namespace framework
