#include "stdafx.h"
#include "Config.h"

#include <rttr/registration>

#include <EtCore/Reflection/Serialization.h>

#include <EtRendering/GlobalRenderingSystems/GlobalRenderingSystems.h>


namespace et {
namespace fw {


// reflection
RTTR_REGISTRATION
{
	using namespace rttr;

	registration::class_<Config::UserDirPointer>("dir pointer")
		.property("user dir path", &Config::UserDirPointer::m_UserDirPath) ;

	registration::class_<Config::Settings::Window>("window")
		.constructor<>()
		.property("title", &Config::Settings::Window::Title)
		.property("fullscreen", &Config::Settings::Window::Fullscreen)
		.property("resolutions", &Config::Settings::Window::Resolutions)
		.property("fullscreen resolution", &Config::Settings::Window::FullscreenRes)
		.property("windowed resolution", &Config::Settings::Window::WindowedRes) ;

	registration::class_<Config::Settings>("settings")
		.constructor<>()
		.property("graphics", &Config::Settings::m_Graphics)
		.property("window", &Config::Settings::m_Window)
		.property("screenshot dir", &Config::Settings::m_ScreenshotDir);
}


//==============================
// Config :: Settings :: Window
//==============================


//------------------------------------
// Config::Settings::Window::GetSize
//
// Retrieves the initial window size as defined by the config data - may be overridden at runtime
//
ivec2 Config::Settings::Window::GetSize() const
{
	size_t const resIdx = Fullscreen ? FullscreenRes : WindowedRes;
	if (resIdx < Resolutions.size())
	{
		return Resolutions[resIdx];
	}

	ET_LOG_W(ET_CTX_FRAMEWORK, "Settings::Window::DeriveSettings > Invalid resolution index in '%s' mode: %u",
		(Fullscreen ? "fullscreen" : "windowed"),
		(Fullscreen ? FullscreenRes : WindowedRes));

	return ivec2();
}


//========
// Config
//========


//---------------------------------
// Config::d-tor
//
Config::~Config()
{
	if (m_HasRenderRef)
	{
		render::RenderingSystems::RemoveReference();
	}
}

//---------------------------------
// Config::Initialize
//
// Load from JSON and generate derived data
//
void Config::Initialize()
{
	// try deserializing the user directory path
	UserDirPointer userDir;
	if (core::serialization::DeserializeFromFile(s_PointerPath, userDir))
	{
		m_UserDir = userDir;
	}
	else
	{
		ET_LOG_E(ET_CTX_FRAMEWORK, "Config::Initialize > unable to deserialize pointer to user directory!");
	}

	// try deserializing settings
	Settings settings;
	if (core::serialization::DeserializeFromFile(GetUserDirPath() + s_ConfigFileRelativePath, settings))
	{
		m_Settings = settings;
	}
	else
	{
		ET_LOG_W(ET_CTX_FRAMEWORK, "Config::Initialize > unable to deserialize config file to settings, using defaults");
	}
}

//---------------------------------
// Config::InitRenderConfig
//
// Pass the graphics settings to the rendering configuration
//
void Config::InitRenderConfig()
{
	if (m_HasRenderRef)
	{
		render::RenderingSystems::Instance()->SetGraphicsSettings(m_Settings.m_Graphics);
	}
	else
	{
		render::RenderingSystems::AddReference(m_Settings.m_Graphics);
		m_HasRenderRef = true;
	}
}

//---------------------------------
// Config::Save
//
// Save the configuration to disk
//
void Config::Save()
{
	if (!core::serialization::SerializeToFile(GetUserDirPath() + s_ConfigFileRelativePath, m_Settings, true))
	{
		ET_LOG_W(ET_CTX_FRAMEWORK, "Config::Save > unable to serialize settings to config file");
	}
}


} // namespace fw
} // namespace et
