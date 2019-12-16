#pragma once

namespace framework {


//====================
// Component Registry
//====================


//-----------------------------
// ComponentRegistry::Register
//
// Register a component type and return its type index
//
template<typename TComponentType>
T_CompTypeIdx ComponentRegistry::Register()
{
	// create type info from the template data
	ComponentTypeInfo ti(rttr::type::get<TComponentType>(), sizeof(TComponentType));

	// ensure we only register components once
	ET_ASSERT(GetTypeIdx(ti.type) == s_InvalidTypeIdx, "Component '%s' was already registered!", ti.type.get_name().data());

	// next free index
	T_CompTypeIdx const idx = static_cast<T_CompTypeIdx>(m_ComponentTypes.size()); 

	m_ComponentTypes.emplace_back(ti);

	return idx;
}


} // namespace framework

