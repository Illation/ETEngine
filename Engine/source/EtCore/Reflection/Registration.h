#pragma once
#include <rttr/registration>
#include <rttr/detail/policies/ctor_policies.h>
#include <rttr/registration_friend> 

// utility macros to make reflecting polymorphic classes less verbose

#define BEGIN_REGISTER_POLYMORPHIC_CLASS(TClass, TName)		\
	rttr::registration::class_<TClass>( TName ).constructor<TClass const&>().constructor<>()(rttr::detail::as_object())

#define REGISTER_POLYMORPHIC_CONVERSION(TDerived, TBase)							\
	rttr::type::register_converter_func([](TDerived& derived, bool& ok) -> TBase*	\
	{																				\
		ok = true;																	\
		return new TDerived(derived);												\
	})

#define END_REGISTER_POLYMORPHIC_CLASS(TDerived, TBase)	\
	;													\
	REGISTER_POLYMORPHIC_CONVERSION(TDerived, TBase)

// allow registering classes with private members within the namespace

#define REGISTRATION_NS(ns)										\
	namespace ns {												\
		static void rttr_auto_register_reflection_function_();	\
	}

#define REGISTRATION_FRIEND_NS(ns)															\
	friend void ns::rttr_auto_register_reflection_function_();                              \
	template<typename Ctor_Type, typename Policy, typename Accessor, typename Arg_Indexer>	\
	friend struct rttr::detail::constructor_invoker;
