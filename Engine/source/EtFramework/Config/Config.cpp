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

	registration::class_<Config::Settings>("settings")
		.constructor<>()
		.property("graphics", &Config::Settings::m_Graphics)
		.property("window", &Config::Settings::m_Window)
		.property("screenshot dir", &Config::Settings::m_ScreenshotDir);
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
