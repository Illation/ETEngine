#pragma once
#include <EtCore/Util/CommonMacros.h>
#include <EtCore/Hashing/HashString.h>


// Debug output (to visual studio)
#ifdef ET_SHIPPING
#	define ET_CT_TRACE_DBG_OUT ET_DISABLED
#else
#	define ET_CT_TRACE_DBG_OUT ET_ENABLED
#endif


// Verbose trace
#ifdef ET_DEBUG
#	define ET_CT_TRACE_VERBOSE ET_ENABLED
#else
#	define ET_CT_TRACE_VERBOSE ET_DISABLED
#endif


namespace et {
namespace core {


//--------------
// E_TraceLevel
//
typedef uint8 T_TraceLevel;
enum E_TraceLevel : T_TraceLevel
{
	TL_Invalid	= 0u,

	TL_Verbose	= 1u << 0,
	TL_Info		= 1u << 1,
	TL_Warning	= 1u << 2,
	TL_Error	= 1u << 3,
	TL_Fatal	= 1u << 5,

	TL_All		= 0xFF
};

typedef HashString T_TraceContext;


} // namespace core
} // namespace et
