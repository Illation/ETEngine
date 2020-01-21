#pragma once


namespace et {
namespace fw {


//=============
// System Base
//=============


namespace detail {

	//-----------------------------
	// SystemTypeListAdder
	//
	// implements a recursive variadic template which adds the type IDs of systems in the template parameters
	//
	template <typename... Args>
	struct SystemTypeListAdder;

	template<typename TSystemType>
	struct SystemTypeListAdder<TSystemType>
	{
		static void Call(T_DependencyList& list)
		{
			list.emplace_back(rttr::type::get<TSystemType>().get_id());
		}
	};

	template<typename TSystemType, typename... Args>
	struct SystemTypeListAdder<TSystemType, Args...>
	{
		static void Call(T_DependencyList& list)
		{
			list.emplace_back(rttr::type::get<TSystemType>().get_id());
			SystemTypeListAdder<Args...>::Call(list);
		}
	};

} // namespace detail

//---------------------------------
// SystemBase::DeclareDependencies
//
// Lists systems that need to execute before this system
//
template<typename... Args>
void SystemBase::DeclareDependencies()
{
	detail::SystemTypeListAdder<Args...>::Call(m_Dependencies);
}

//-------------------------------
// SystemBase::DeclareDependents
//
// Lists systems that need to be executed after this system
//
template<typename... Args>
void SystemBase::DeclareDependents()
{
	detail::SystemTypeListAdder<Args...>::Call(m_Dependents);
}


//========
// System
//========


//-------------------
// System::GetTypeId
//
template <class TSystemType, typename TViewType>
T_SystemType fw::System<TSystemType, TViewType>::GetTypeId() const
{
	return rttr::type::get<TSystemType>().get_id();
}

//---------------------
// System::GetSignature
//
template <class TSystemType, typename TViewType>
ComponentSignature fw::System<TSystemType, TViewType>::GetSignature() const
{
	return SignatureFromView<TViewType>();
}

//---------------------
// System::RootProcess
//
// Implements the low level accessible base process method by generating a range for the derived system class to use
//
template <class TSystemType, typename TViewType>
void System<TSystemType, TViewType>::RootProcess(EcsController* const control, Archetype* const archetype, size_t const offset, size_t const count) 
{
	Process(ComponentRange<TViewType>(control, archetype, offset, count));
}
	

} // namespace fw
} // namespace et
