#include <EtCore/stdafx.h>
#include "ContentBuildConfiguration.h"


namespace et {
namespace pl {


// reflection
RTTR_REGISTRATION
{
	rttr::registration::enumeration<et::pl::BuildConfiguration::E_Configuration>("E_Configuration") (
		rttr::value("Debug", et::pl::BuildConfiguration::E_Configuration::Debug),
		rttr::value("Develop", et::pl::BuildConfiguration::E_Configuration::Develop),
		rttr::value("Shipping", et::pl::BuildConfiguration::E_Configuration::Shipping));

	rttr::registration::enumeration<et::pl::BuildConfiguration::E_Architecture>("E_Architecture") (
		rttr::value("x32", et::pl::BuildConfiguration::E_Architecture::x32),
		rttr::value("x64", et::pl::BuildConfiguration::E_Architecture::x64));

	rttr::registration::enumeration<et::pl::BuildConfiguration::E_Platform>("E_Platform") (
		rttr::value("Windows", et::pl::BuildConfiguration::E_Platform::Windows),
		rttr::value("Linux", et::pl::BuildConfiguration::E_Platform::Linux));

	rttr::registration::enumeration<et::pl::BuildConfiguration::E_GraphicsBackend>("E_GraphicsBackend") (
		rttr::value("OpenGL", et::pl::BuildConfiguration::E_GraphicsBackend::OpenGL));
}

//------------------------------
// BuilConfiguration::c-tor
//
// Default construct based on source build
//
BuildConfiguration::BuildConfiguration()
{
#ifdef ET_DEBUG
	m_Configuration = E_Configuration::Debug;
#else 
#ifdef ET_DEVELOP
	m_Configuration = E_Configuration::Develop;
#else
#ifdef ET_SHIPPING
	m_Configuration = E_Configuration::Shipping;
#else
	ET_ERROR("No configuration preprocessor defined");
#endif
#endif
#endif

#ifdef ET_ARCH_X32
	m_Architecture = E_Architecture::x32;
#else 
#ifdef ET_ARCH_X64
	m_Architecture = E_Architecture::x64;
#else
	ET_ERROR("No architecture preprocessor defined");
#endif
#endif

#ifdef ET_PLATFORM_WIN
	m_Platform = E_Platform::Windows;
#else 
#ifdef ET_PLATFORM_LINUX
	m_Platform = E_Platform::Linux;
#else
	ET_ERROR("No platform preprocessor defined");
#endif
#endif
}

//------------------------------
// BuilConfiguration::c-tor
//
BuildConfiguration::BuildConfiguration(E_Configuration const config, E_Architecture const arch, E_Platform const platform) 
	: m_Configuration(config)
	, m_Architecture(arch)
	, m_Platform(platform)
{ }


} // namespace pl
} // namespace et
