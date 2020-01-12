#pragma once
#include "ComponentRegistry.h"
#include "ComponentRange.h"
#include "EcsCommandBuffer.h"

#include <rttr/type.h>


namespace fw {


// fwd
class Archetype;


// def
typedef rttr::type::type_id T_SystemType;
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
	virtual T_SystemType GetTypeId() const = 0;
	virtual ComponentSignature GetSignature() const = 0;

	// the important one
	virtual void RootProcess(EcsController* const controller, Archetype* const archetype, size_t const offset, size_t const count) = 0; 

	// functionality
	//---------------
	void SetCommandController(EcsController* const ecs) { m_CommandBuffer.SetController(ecs); }
	void MergeCommands() { m_CommandBuffer.Merge(); }

	// accessors
	//-----------
	T_DependencyList const& GetDependencies() const { return m_Dependencies; }
	T_DependencyList const& GetDependents() const { return m_Dependents; }

	EcsCommandBuffer& GetCommandBuffer() { return m_CommandBuffer; }

	// utility - use these in system constructor
	//-------------------------------------------
protected:
	template<typename... Args>
	void DeclareDependencies();
	template<typename... Args>
	void DeclareDependents();

	// Data
	///////

	EcsCommandBuffer m_CommandBuffer;

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
	T_SystemType GetTypeId() const override;
	ComponentSignature GetSignature() const override;

	void RootProcess(EcsController* const controller, Archetype* const archetype, size_t const offset, size_t const count) override;

	// interface
	//-----------
	virtual void Process(ComponentRange<TViewType>& range) = 0;
};


} // namespace fw


#include "System.inl"
