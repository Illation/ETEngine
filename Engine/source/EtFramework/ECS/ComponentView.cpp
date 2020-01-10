#include "stdafx.h"
#include "Archetype.h"
#include "ComponentView.h"
#include "EcsController.h"


namespace fw {


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

	// set the pointers to the position within the pool that the range starts from
	for (Accessor& access : m_Accessors)
	{
		access.currentElement = static_cast<uint8*>(m_Range->m_Archetype->GetPool(access.typeIdx).At(m_Range->m_Offset));
	}

	// set the ecs controllers for accessors outside the current entity
	for (EcsController const** controllerPtr : m_ControllerPtrs)
	{
		*controllerPtr = m_Range->m_Controller;
	}

	CalcParentPointers();
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
	ret.reserve(m_Accessors.size() + m_Includes.size());

	for (Accessor const& access : m_Accessors)
	{
		ret.emplace_back(access.typeIdx);
	}

	ret.insert(ret.end(), m_Includes.begin(), m_Includes.end());

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

	// components in our entity
	for (Accessor& access : m_Accessors)
	{
		access.currentElement += ComponentRegistry::Instance().GetSize(access.typeIdx);
	}

	CalcParentPointers();

	return false;
}

//-----------------------------------
// ComponentView::CalcParentPointers
//
// Set pointers to the parent entities components based on the current entity
//
void ComponentView::CalcParentPointers()
{
	if (!m_ParentAccessors.empty())
	{
		T_EntityId const current = m_Range->m_Archetype->GetEntity(m_Current + m_Range->m_Offset);
		T_EntityId const parent = m_Range->m_Controller->GetParent(current);
		if (parent != INVALID_ENTITY_ID)
		{
			for (Accessor& access : m_ParentAccessors)
			{
				if (m_Range->m_Controller->HasComponent(parent, access.typeIdx))
				{
					access.currentElement = static_cast<uint8*>(m_Range->m_Controller->GetComponentData(parent, access.typeIdx));
				}
				else
				{
					access.currentElement = nullptr;
				}
			}
		}
		else
		{
			for (Accessor& access : m_ParentAccessors)
			{
				access.currentElement = nullptr;
			}
		}
	}
}


} // namespace fw
