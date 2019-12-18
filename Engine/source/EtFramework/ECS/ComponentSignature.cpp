#include "stdafx.h"
#include "ComponentSignature.h"


namespace framework {


//=====================
// Component Signature
//=====================


//---------------------------
// ComponentSignature::c-tor
//
ComponentSignature::ComponentSignature(T_CompTypeList const& types)
	: m_Impl(types)
{
	std::sort(m_Impl.begin(), m_Impl.end());
}

//-----------------------------------------
// ComponentSignature::GetMaxComponentType
//
T_CompTypeIdx ComponentSignature::GetMaxComponentType() const
{
	if (m_Impl.size() == 0u)
	{
		return INVALID_COMP_TYPE_IDX;
	}

	return m_Impl[m_Impl.size() - 1];
}

//-----------------------------------------------
// ComponentSignature::MatchesComponentsUnsorted
//
bool ComponentSignature::MatchesComponentsUnsorted(std::vector<RawComponentData> const& list) const
{
	if (m_Impl.size() != list.size())
	{
		return false;
	}

	for (RawComponentData const& data : list)
	{
		if (std::find(m_Impl.cbegin(), m_Impl.cend(), data.typeIdx) == m_Impl.cend())
		{
			return false;
		}
	}

	return true;
}

//-------------------------------
// ComponentSignature comparison
//
bool operator==(ComponentSignature const& lhs, ComponentSignature const& rhs)
{
	return std::equal(lhs.GetTypes().cbegin(), lhs.GetTypes().cend(), rhs.GetTypes().cbegin());
}


} // namespace framework
