#include "stdafx.h"
#include "ComponentRange.h"
#include "Archetype.h"


namespace fw {


//======================
// Base Component Range
//======================


//---------------------------
// BaseComponentRange::c-tor
//
BaseComponentRange::BaseComponentRange(EcsController* const controller, Archetype* const archetype, size_t const offset, size_t const count)
	: m_Controller(controller)
	, m_Archetype(archetype)
	, m_Offset(offset)
	, m_Count(count)
{
	ET_ASSERT(m_Archetype != nullptr);
	ET_ASSERT(offset + count <= m_Archetype->GetSize());
}


} // namespace fw
