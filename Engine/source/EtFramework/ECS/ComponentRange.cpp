#include "stdafx.h"
#include "ComponentRange.h"
#include "Archetype.h"


namespace framework {


//======================
// Base Component Range
//======================


//---------------------------
// BaseComponentRange::c-tor
//
BaseComponentRange::BaseComponentRange(Archetype* const archetype, size_t const offset, size_t const count)
	: m_Archetype(archetype)
	, m_Offset(offset)
	, m_Count(count)
{
	ET_ASSERT(m_Archetype != nullptr);
	ET_ASSERT(offset + count <= m_Archetype->GetSize());
}


} // namespace framework
