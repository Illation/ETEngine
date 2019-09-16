#include "stdafx.h"
#include "Config.h"

#include <EtCore/Reflection/Serialization.h>

#include <rttr/registration>


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
		.property("num cascades", &Config::Settings::Graphics::NumCascades)
		.property("csm draw distance", &Config::Settings::Graphics::CSMDrawDistance)
		.property("num PCF samples", &Config::Settings::Graphics::NumPCFSamples)
		.property("BRDF LUT size", &Config::Settings::Graphics::PbrBrdfLutSize)
		.property("device id", &Config::Settings::Graphics::TextureScaleFactor)
		.property("device id", &Config::Settings::Graphics::NumBlurPasses)
		;

	registration::class_<Config::Settings::Window>("window")
		.constructor<>()
		.method("DeriveSettings", &Config::Settings::Window::DeriveSettings)
		.method("Resize", &Config::Settings::Window::Resize)
		.property("title", &Config::Settings::Window::Title)
		.property("fullscreen", &Config::Settings::Window::Fullscreen)
		.property("dimensions", &Config::Settings::Window::Dimensions)
		;

	registration::class_<Config::Settings>("settings")
		.constructor<>()
		.property("graphics", &Config::Settings::m_Graphics)
		.property("window", &Config::Settings::m_Window)
		;
}

//-------------------------------------------
// Config::Settings::Window::GetAspectRatio
//
// Derive some settings from those that where loaded from json
//
void Config::Settings::Window::DeriveSettings()
{
	AspectRatio = static_cast<float>(Width) / static_cast<float>(Height);
}

//------------------------------------
// Config::Settings::Window::Resize
//
// Resizes the window and notifies listeners
//
void Config::Settings::Window::Resize(int32 width, int32 height, bool broadcast)
{
	Width = width;
	Height = height;

	DeriveSettings();

	if (broadcast)
	{
		WindowResizeEvent.Broadcast();
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
	if (serialization::DeserializeFromFile(s_PointerPath, userDir))
	{
		m_UserDir = userDir;
	}
	else
	{
		LOG("Config::Initialize > unable to deserialize pointer to user directory, using default user directory: '"
			+ std::string(s_DefaultUserDir) + std::string("'"), Warning);
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

	// derive settings regardless of whether they where loaded or default
	m_Settings.m_Window.DeriveSettings();
}

//---------------------------------
// Config::Initialize
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
