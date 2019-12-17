#pragma once
#include "ComponentRegistry.h"

#include <rttr/type.h>


namespace framework {


// fwd
class Archetype;


//---------------
// E_ComponentAccess
//
// Abstract implementation of a system
//
enum class E_ComponentAccess : uint8
{
	Read,
	ReadWrite,
	Exclude,
	Undefined
};


//---------------
// SystemBase
//
// Abstract implementation of a system
//
class SystemBase
{
	// construct destruct
	//--------------------
public:
	SystemBase() = default;
	virtual ~SystemBase() = default;

	// interface
	//-----------
	virtual rttr::type const& GetType() const = 0;

	virtual void Process(Archetype* const archetype, size_t const offset, size_t const count) const = 0; // the important one

	// accessors
	//-----------
	bool MatchesSignature(std::vector<T_CompTypeIdx> const& sig) const;
	std::vector<T_CompTypeIdx> const& GetSignature() const { return m_Signature; }
	E_ComponentAccess GetAccessType(T_CompTypeIdx const typeIdx) const;

	std::vector<rttr::type> const& GetDependencies() const { return m_Dependencies; }
	std::vector<rttr::type> const& GetDependents() const { return m_Dependents; }

	// utility - use these in system constructor
	//-------------------------------------------
protected:
	template<typename TComponentType>
	void DeclareComponentAccess(E_ComponentAccess const access);
	template<typename TComponentType, typename... Args>
	void DeclareComponentAccess(E_ComponentAccess const access);

	void DeclareDependencies(std::vector<rttr::type> const& dep);
	void DeclareDependents(std::vector<rttr::type> const& dep);

	// Data
	///////

private:
	std::vector<T_CompTypeIdx> m_Signature;
	std::vector<E_ComponentAccess> m_Access;

	std::vector<rttr::type> m_Dependencies;
	std::vector<rttr::type> m_Dependents;
};


//---------------
// System
//
// Use CRTP to infer the type method - create new systems by inheriting: 
//  class MySystem : public System<MySystem>
//
template <class TSystemType>
class System : public SystemBase
{
public:
	System() : SystemBase() {}
	virtual ~System() {}

	rttr::type const& GetType() const override { return rttr::type::get<TSystemType>(); }
};


} // namespace framework
