#include "stdafx.h"
#include "ComponentSignature.h"


namespace framework {


//=====================
// Component Signature
//=====================


//---------------------------
// ComponentSignature::c-tor
//
// construct from type list
//
ComponentSignature::ComponentSignature(T_CompTypeList const& types)
	: m_Impl(types)
{
	std::sort(m_Impl.begin(), m_Impl.end());
}

//---------------------------
// ComponentSignature::c-tor
//
// construct from component list
//
ComponentSignature::ComponentSignature(std::vector<RawComponentPtr> const& components)
{
	for (RawComponentPtr const& comp : components)
	{
		m_Impl.push_back(comp.typeIdx);
	}

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
bool ComponentSignature::MatchesComponentsUnsorted(std::vector<RawComponentPtr> const& list) const
{
	if (m_Impl.size() != list.size())
	{
		return false;
	}

	for (RawComponentPtr const& data : list)
	{
		if (std::find(m_Impl.cbegin(), m_Impl.cend(), data.typeIdx) == m_Impl.cend())
		{
			return false;
		}
	}

	return true;
}

//--------------------------------
// ComponentSignature::Contains
//
bool ComponentSignature::Contains(ComponentSignature const& other) const
{
	if (other.GetSize() > m_Impl.size()) // there is definitly a type in the other signature that we don't have
	{
		return false;
	}

	T_CompTypeList const& otherTypes = other.GetTypes();

	// since both lists are sorted we can just iterate linearly over the smaller list and our inner list at the same time
	size_t idx = 0;
	for (T_CompTypeIdx const otherType : otherTypes) 
	{
		T_CompTypeIdx thisType = INVALID_COMP_TYPE_IDX; // iterate our inner list until we find the matching component
		do
		{
			if (idx >= m_Impl.size()) // if we reach the end of our list the comparison failed
			{
				return false;
			}

			thisType = m_Impl[idx];

			if (thisType > otherType) // if our type is larger, the matching type won't be found as both lists are sorted
			{
				return false;
			}

			++idx;
		} 
		while (thisType != otherType);
	}

	return true;
}

//----------------------------
// ComponentSignature::GenId
//
T_Hash ComponentSignature::GenId() const
{
	return GetDataHash(reinterpret_cast<uint8 const*>(m_Impl.data()), m_Impl.size() * sizeof(T_CompTypeIdx));
}

//-------------------------------
// ComponentSignature comparison
//
bool operator==(ComponentSignature const& lhs, ComponentSignature const& rhs)
{
	return std::equal(lhs.GetTypes().cbegin(), lhs.GetTypes().cend(), rhs.GetTypes().cbegin());
}


} // namespace framework
