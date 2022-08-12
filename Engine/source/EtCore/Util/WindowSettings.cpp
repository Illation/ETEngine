#include "stdafx.h"
#include "WindowSettings.h"

#include <rttr/registration>


namespace et {
namespace core {


//================
// WindowSettings
//================


// reflection
RTTR_REGISTRATION
{
	using namespace rttr;

	registration::class_<WindowSettings>("window")
		.constructor<>()
		.property("title", &WindowSettings::m_Title)
		.property("fullscreen", &WindowSettings::m_Fullscreen)
		.property("resolutions", &WindowSettings::m_Resolutions)
		.property("fullscreen resolution", &WindowSettings::m_FullscreenRes)
		.property("windowed resolution", &WindowSettings::m_WindowedRes);
}


//------------------------------------
// Config::Settings::WindowSettings::GetSize
//
// Retrieves the initial window size as defined by the config data - may be overridden at runtime
//
ivec2 WindowSettings::GetSize() const
{
	size_t const resIdx = m_Fullscreen ? m_FullscreenRes : m_WindowedRes;
	if (resIdx < m_Resolutions.size())
	{
		return m_Resolutions[resIdx];
	}

	ET_WARNING("Settings::WindowSettings::DeriveSettings > Invalid resolution index in '%s' mode: %u",
		(m_Fullscreen ? "fullscreen" : "windowed"),
		(m_Fullscreen ? m_FullscreenRes : m_WindowedRes));

	return ivec2();
}


} // namespace core
} // namespace et
