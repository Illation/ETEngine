#pragma once
#include <EtCore/Util/CommonMacros.h>


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
