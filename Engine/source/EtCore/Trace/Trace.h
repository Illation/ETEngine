#pragma once
#include "TraceService.h"

#include <EtCore/Util/StringUtil.h>


// Trace vs just logs
#ifdef ET_SHIPPING
#	define ET_CT_TRACE ET_DISABLED
#else
#	define ET_CT_TRACE ET_ENABLED
#endif


// Logging is always enabled, even in release builds

#define ET_LOG(context, level, timestamp, ...) et::core::TraceService::Instance()->Trace(context::s_Id, level, timestamp, FS(__VA_ARGS__))

#define ET_LOG_V(context, ...) ET_LOG(context, et::core::E_TraceLevel::TL_Verbose, false, __VA_ARGS__)
#define ET_LOG_I(context, ...) ET_LOG(context, et::core::E_TraceLevel::TL_Info, false, __VA_ARGS__)
#define ET_LOG_W(context, ...) ET_LOG(context, et::core::E_TraceLevel::TL_Warning, false, __VA_ARGS__)
#define ET_LOG_E(context, ...) ET_LOG(context, et::core::E_TraceLevel::TL_Error, false, __VA_ARGS__)


// trace code can be disabled at compile time

#if ET_CT_IS_ENABLED(ET_CT_TRACE)

#define ET_TRACE(context, level, timestamp, ...) ET_LOG(context, level, timestamp, __VA_ARGS__)

#define ET_TRACE_V(context, ...) ET_LOG_V(context, __VA_ARGS__)
#define ET_TRACE_I(context, ...) ET_LOG_I(context, __VA_ARGS__)
#define ET_TRACE_W(context, ...) ET_LOG_W(context, __VA_ARGS__)
#define ET_TRACE_E(context, ...) ET_LOG_E(context, __VA_ARGS__)

#else

#define ET_TRACE(context, level, timestamp, ...)

#define ET_TRACE_V(context, ...) 
#define ET_TRACE_I(context, ...) 
#define ET_TRACE_W(context, ...) 
#define ET_TRACE_E(context, ...) 

#endif // ET_CT_IS_ENABLED(ET_CT_TRACE)
