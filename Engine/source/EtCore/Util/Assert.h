#pragma once
#include <EtCore/Util/Logger.h>

#include "CommonMacros.h"
#include "StringUtil.h"


#ifndef ET_SHIPPING
#ifdef ET_PLATFORM_WIN
#	define ET_PROCESS_ASSERT_IMPL(condition, ...) et::core::Logger::ProcessAssert(condition, __FUNCSIG__, FS(__VA_ARGS__))
#else
#	define ET_PROCESS_ASSERT_IMPL(condition, ...) et::core::Logger::ProcessAssert(condition, __PRETTY_FUNCTION__, FS(__VA_ARGS__))
#endif // ET_PLATFORM_WIN														

#	define ET_ASSERT_IMPL(condition, ...)\
	__pragma(warning(push))\
	__pragma(warning(disable: 4127))\
	do\
	{\
		if (ET_PROCESS_ASSERT_IMPL(condition, __VA_ARGS__)) ET_BREAK();\
	}\
	while (false)\
	__pragma(warning(pop))
#endif // ET_SHIPPING

#ifdef ET_SHIPPING
#	define ET_ASSERT(condition, ...)
#else
#	define ET_ASSERT(condition, ...) ET_ASSERT_IMPL(condition, __VA_ARGS__)
#endif // ET_SHIPPING

// for performance critical code
#define ET_PARANOID_ASSERTS_ENABLED 0

#if ET_PARANOID_ASSERTS_ENABLED
#	define ET_ASSERT_PARANOID(condition, ...) ET_ASSERT(condition, __VA_ARGS__)
#else
#	define ET_ASSERT_PARANOID(condition, ...)
#endif

