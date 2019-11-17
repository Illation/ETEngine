#include "stdafx.h"
#include "Config.h"

#include <EtCore/Reflection/Serialization.h>

#include <rttr/registration>


//========
// Config
//========


// reflection
RTTR_REGISTRATION
{
	using namespace rttr;

	registration::class_<Config::UserDirPointer>("dir pointer")
		.property("user dir path", &Config::UserDirPointer::m_UserDirPath)
		;

	registration::class_<Config::Settings::Graphics>("output")
		.constructor<>()
		.property("use FXAA", &Config::Settings::Graphics::UseFXAA)
		.property("CSM cascade count", &Config::Settings::Graphics::NumCascades)
		.property("CSM draw distance", &Config::Settings::Graphics::CSMDrawDistance)
		.property("PCF sample count", &Config::Settings::Graphics::NumPCFSamples)
		.property("BRDF LUT size", &Config::Settings::Graphics::PbrBrdfLutSize)
		.property("texture scale factor", &Config::Settings::Graphics::TextureScaleFactor)
		.property("bloom blur passes", &Config::Settings::Graphics::NumBlurPasses)
		;

	registration::class_<Config::Settings::Window>("window")
		.constructor<>()

		.property("title", &Config::Settings::Window::Title)
		.property("fullscreen", &Config::Settings::Window::Fullscreen)
		.property("resolutions", &Config::Settings::Window::Resolutions)
		.property("fullscreen resolution", &Config::Settings::Window::FullscreenRes)
		.property("windowed resolution", &Config::Settings::Window::WindowedRes)
		;

	registration::class_<Config::Settings>("settings")
		.constructor<>()
		.property("graphics", &Config::Settings::m_Graphics)
		.property("window", &Config::Settings::m_Window)
		.property("start scene", &Config::Settings::m_StartScene)
		.property("screenshot dir", &Config::Settings::m_ScreenshotDir)
		;
}

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

	LOG(FS("Settings::Window::DeriveSettings > Invalid resolution index in '%s' mode: %u",
		(Fullscreen ? "fullscreen" : "windowed"),
		(Fullscreen ? FullscreenRes : WindowedRes)),
		LogLevel::Warning);

	return ivec2();
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
	if (serialization::DeserializeFromFile(s_PointerPath, userDir))
	{
		m_UserDir = userDir;
	}
	else
	{
		LOG("Config::Initialize > unable to deserialize pointer to user directory!", LogLevel::Error);
	}

	// try deserializing settings
	Settings settings;
	if (serialization::DeserializeFromFile(GetUserDirPath() + s_ConfigFileRelativePath, settings))
	{
		m_Settings = settings;
	}
	else
	{
		LOG("Config::Initialize > unable to deserialize config file to settings, using defaults", Warning);
	}
}

//---------------------------------
// Config::Save
//
// Save the configuration to disk
//
void Config::Save()
{
	if (!serialization::SerializeToFile(GetUserDirPath() + s_ConfigFileRelativePath, m_Settings))
	{
		LOG("Config::Save > unable to serialize settings to config file", Warning);
	}
}
