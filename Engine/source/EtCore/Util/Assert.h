#pragma once
#include "CommonMacros.h"
#include "StringUtil.h"


// when is it enabled
//====================

#ifdef ET_SHIPPING
#	define ET_CT_ASSERT ET_DISABLED
#else
#	define ET_CT_ASSERT ET_ENABLED
#endif // ET_SHIPPING



// implementation
//=================


#if ET_CT_IS_ENABLED(ET_CT_ASSERT)

namespace et { namespace detail {
	bool ProcessAssert(bool const condition, std::string const& caller, std::string const& msg);
} } 


#	define ET_ASSERT_HANDLER_DEFAULT() ET_BREAK();


#	ifdef ET_PLATFORM_WIN
#		define ET_PROCESS_ASSERT_IMPL(condition, ...) et::detail::ProcessAssert(condition, __FUNCSIG__, FS(__VA_ARGS__))
#	else
#		define ET_PROCESS_ASSERT_IMPL(condition, ...) et::detail::ProcessAssert(condition, __PRETTY_FUNCTION__, FS(__VA_ARGS__))
#	endif // ET_PLATFORM_WIN														


#	define ET_ASSERT_IMPL(condition, ...)\
	__pragma(warning(push))\
	__pragma(warning(disable: 4127))\
	do\
	{\
		if (ET_PROCESS_ASSERT_IMPL(condition, __VA_ARGS__)) ET_ASSERT_HANDLER_DEFAULT();\
	}\
	while (false)\
	__pragma(warning(pop))

#endif // ET_CT_ASSERT



// assert macros
//=================


#if ET_CT_IS_ENABLED(ET_CT_ASSERT)
#	define ET_ASSERT(condition, ...) ET_ASSERT_IMPL(condition, __VA_ARGS__)
#else
#	define ET_ASSERT(condition, ...)
#endif // ET_CT_ASSERT

// for performance critical code
#define ET_CT_PARANOID_ASSERTS ET_DISABLED

#if ET_CT_IS_ENABLED(ET_CT_PARANOID_ASSERTS)
#	define ET_ASSERT_PARANOID(condition, ...) ET_ASSERT(condition, __VA_ARGS__)
#else
#	define ET_ASSERT_PARANOID(condition, ...)
#endif

