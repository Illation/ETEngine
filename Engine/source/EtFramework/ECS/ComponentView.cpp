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
ComponentView::Accessor::Accessor(void*& pointerRef, T_CompTypeIdx const type, bool const readAccess) 
	: currentElement(reinterpret_cast<uint8*&>(pointerRef))
	, typeIdx(type)
	, read(readAccess)
{ } 


//================
// Component View
//================


//----------------------
// ComponentView::c-tor
//
// initialize references to all pointers
//
ComponentView::ComponentView(BaseComponentRange* const range) 
	: m_Max(range->m_Count)
{
	Register(); 

	// set the pointers to the position within the pool that the range starts from
	for (Accessor& access : m_Accessors)
	{
		access.currentElement = static_cast<uint8*>(range->m_Archetype->GetPool(access.typeIdx).At(range->m_Offset));
	}
}

//---------------------
// ComponentView::Next
//
// Increment component pointers
//
bool ComponentView::Next()
{
	m_Current++;
	if (m_Current >= m_Max)
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
