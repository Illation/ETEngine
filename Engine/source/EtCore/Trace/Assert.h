#pragma once
#include <EtCore/Util/CommonMacros.h>
#include <EtCore/Util/StringUtil.h>

#include "TraceFwd.h"


// when is it enabled
//====================

// ASSERT

#ifdef ET_SHIPPING
#	define ET_CT_ASSERT ET_DISABLED
#else
#	define ET_CT_ASSERT ET_ENABLED
#endif // ET_SHIPPING


// ASSERT_PARANOID
// for performance critical code

#define ET_CT_PARANOID_ASSERTS ET_DISABLED


// WARNING
// unconditional warning and break

#ifdef ET_SHIPPING
#	define ET_CT_WARNING ET_DISABLED
#else
#	define ET_CT_WARNING ET_ENABLED
#endif // ET_SHIPPING


// ERROR
// unconditional error and break

#ifdef ET_SHIPPING
#	define ET_CT_ERROR ET_DISABLED
#else
#	define ET_CT_ERROR ET_ENABLED
#endif // ET_SHIPPING


// FATAL
// unconditional error and exit

#define ET_CT_FATAL ET_ENABLED


// implementation
//=================


#if ET_CT_IS_ENABLED(ET_CT_ASSERT)

namespace et { namespace detail {
	bool ProcessAssert(bool const condition, std::string const& caller, std::string const& msg);
	void ProcessReport(core::E_TraceLevel const level, std::string const& caller, std::string const& msg);
	void FatalHandler(core::E_TraceLevel const level);
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


#	ifdef ET_PLATFORM_WIN
#		define ET_PROCESS_REPORT_IMPL(level, ...) et::detail::ProcessReport(level, __FUNCSIG__, FS(__VA_ARGS__))
#	else
#		define ET_PROCESS_REPORT_IMPL(level, ...) et::detail::ProcessReport(level, __PRETTY_FUNCTION__, FS(__VA_ARGS__))
#	endif // ET_PLATFORM_WIN		

#	define ET_REPORT_IMPL(level, ...)\
	__pragma(warning(push))\
	__pragma(warning(disable: 4127))\
	do\
	{\
		ET_PROCESS_REPORT_IMPL(level, __VA_ARGS__);\
		ET_ASSERT_HANDLER_DEFAULT();\
		et::detail::FatalHandler(level);\
	}\
	while (false)\
	__pragma(warning(pop))

#endif // ET_CT_ASSERT



// assert macros
//=================


// to check some state before relying on it
#if ET_CT_IS_ENABLED(ET_CT_ASSERT)
#	define ET_ASSERT(condition, ...) ET_ASSERT_IMPL(condition, __VA_ARGS__)
#else
#	define ET_ASSERT(condition, ...)
#endif 


// usually disabled checks for performance critical code
#if ET_CT_IS_ENABLED(ET_CT_PARANOID_ASSERTS)
#	define ET_ASSERT_PARANOID(condition, ...) ET_ASSERT_IMPL(condition, __VA_ARGS__)
#else
#	define ET_ASSERT_PARANOID(condition, ...)
#endif 


// for probably recoverable problems
#if ET_CT_IS_ENABLED(ET_CT_WARNING)
#	define ET_WARNING(...) ET_REPORT_IMPL(et::core::E_TraceLevel::TL_Warning, __VA_ARGS__)
#else
#	define ET_WARNING(...)
#endif 


// for possibly breaking problems
#if ET_CT_IS_ENABLED(ET_CT_ERROR)
#	define ET_ERROR(...) ET_REPORT_IMPL(et::core::E_TraceLevel::TL_Error, __VA_ARGS__)
#else
#	define ET_ERROR(...)
#endif 


// for breaking problems that make it better to crash the app than allowing it to keep running
#if ET_CT_IS_ENABLED(ET_CT_FATAL)
#	define ET_FATAL(...) ET_REPORT_IMPL(et::core::E_TraceLevel::TL_Fatal, __VA_ARGS__)
#else
#	define ET_FATAL(...)
#endif 

