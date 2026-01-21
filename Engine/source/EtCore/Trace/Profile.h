#pragma once

// Profiling compile switches
#if defined(ET_DEBUG) || defined(ET_SHIPPING)
#	define ET_CT_PROFILE ET_DISABLED
#else
#	define ET_CT_PROFILE ET_ENABLED
#endif

#if ET_CT_IS_ENABLED(ET_CT_PROFILE) && defined(ET_PROFILING)
#	define ET_CT_PROFILE_FINE ET_ENABLED
#else
#	define ET_CT_PROFILE_FINE ET_DISABLED
#endif

#if ET_CT_IS_ENABLED(ET_CT_PROFILE)

TRACY_ENABLE
#include <Tracy.hpp>


#define ET_ZoneNamed(varname, active)                 ZoneNamed(varname, active)
#define ET_ZoneNamedN(varname, name, active)          ZoneNamedN(varname, name, active)
#define ET_ZoneNamedC(varname, color, active)         ZoneNamedC(varname, color, active)
#define ET_ZoneNamedNC(varname, name, color, active)  ZoneNamedNC(varname, name, color, active)

#define ET_ZoneScoped                 ZoneScoped
#define ET_ZoneScopedN(name)          ZoneScopedN(name)
#define ET_ZoneScopedC(color)         ZoneScopedC(color)
#define ET_ZoneScopedNC(name, color)  ZoneScopedNC(name, color)

#define ET_ZoneText(txt, size)        ZoneText(txt, size)
#define ET_ZoneName(txt, size)        ZoneName(txt, size)

#define ET_TracyPlot(name,val)        TracyPlot(name, val)

#define ET_FrameMark                  FrameMark
#define ET_FrameMarkNamed(name)       FrameMarkNamed(name)
#define ET_FrameMarkStart(name)       FrameMarkStart(name)
#define ET_FrameMarkEnd(name)         FrameMarkEnd(name)

#else

#define ET_ZoneNamed(varname, active)
#define ET_ZoneNamedN(varname, name, active)
#define ET_ZoneNamedC(varname, color, active)
#define ET_ZoneNamedNC(varname, name, color, active)

#define ET_ZoneScoped
#define ET_ZoneScopedN(name)
#define ET_ZoneScopedC(color)
#define ET_ZoneScopedNC(name, color)

#define ET_ZoneText(txt, size)
#define ET_ZoneName(txt, size)

#define ET_TracyPlot(name,val)

#define ET_FrameMark
#define ET_FrameMarkNamed(name)
#define ET_FrameMarkStart(name)
#define ET_FrameMarkEnd(name)

#endif // ET_CT_IS_ENABLED(ET_CT_TRACE)

