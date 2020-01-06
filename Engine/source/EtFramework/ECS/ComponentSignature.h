#pragma once
#include "ComponentRegistry.h"
#include "RawComponentPointer.h"


namespace fw {


//--------------------
// ComponentSignature
//
// Identifier for a component composition
//
class ComponentSignature final
{
	// construct destruct
	//--------------------
public:
	ComponentSignature(T_CompTypeList const& types);
	ComponentSignature(std::vector<RawComponentPtr> const& components);

	// accessors
	//-----------    
	T_CompTypeList const& GetTypes() const { return m_Impl; }
	size_t GetSize() const { return m_Impl.size(); }
	T_CompTypeIdx GetMaxComponentType() const;
	bool MatchesComponentsUnsorted(std::vector<RawComponentPtr> const& list) const;
	bool Contains(ComponentSignature const& other) const;
	T_Hash GenId() const;

	// Data
	///////

private:
	T_CompTypeList m_Impl;
};


// comparison
bool operator == (ComponentSignature const& lhs, ComponentSignature const& rhs);


// generation
template<typename... Args>
ComponentSignature GenSignature() { return ComponentSignature(GenCompTypeList<Args...>()); }


} // namespace fw
