#pragma once
#include "ComponentRegistry.h"
#include "ComponentRange.h"

#include <rttr/type.h>


namespace framework {


// fwd
class Archetype;


// def
typedef std::vector<rttr::type::type_id> T_DependencyList;


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
	virtual rttr::type::type_id GetTypeId() const = 0;
	virtual ComponentSignature GetSignature() const = 0;

	virtual void RootProcess(Archetype* const archetype, size_t const offset, size_t const count) const = 0; // the important one

	// accessors
	//-----------
	T_DependencyList const& GetDependencies() const { return m_Dependencies; }
	T_DependencyList const& GetDependents() const { return m_Dependents; }

	// utility - use these in system constructor
	//-------------------------------------------
protected:
	template<typename... Args>
	void DeclareDependencies();
	template<typename... Args>
	void DeclareDependents();

	// Data
	///////

private:
	T_DependencyList m_Dependencies;
	T_DependencyList m_Dependents;
};


//---------------
// System
//
// Use CRTP to infer the type method - create new systems by inheriting: 
//  class MySystem : public System<MySystem, MyComponentView>
//
template <class TSystemType, typename TViewType>
class System : public SystemBase
{
	// construct destruct
	//--------------------
public:
	System() : SystemBase() {}
	virtual ~System() = default;

	// System Base interface implementation
	//--------------------------------------
	rttr::type::type_id GetTypeId() const override;
	ComponentSignature GetSignature() const override;

	void RootProcess(Archetype* const archetype, size_t const offset, size_t const count) const override;

	// interface
	//-----------
	virtual void Process(ComponentRange<TViewType>& range) const = 0;
};


} // namespace framework


#include "System.inl"
