#include "stdafx.h"
#include "BootConfig.h"

#include <EtCore/FileSystem/Package/Package.h>
#include <EtCore/Reflection/Registration.h>
#include <EtCore/Reflection/JsonDeserializer.h>


namespace et {
namespace fw {


//=============
// Boot Config
//=============


// reflection
RTTR_REGISTRATION
{
	rttr::registration::class_<BootConfig>("boot config")
		.property("start scene", &BootConfig::startScene)
		.property("all scenes", &BootConfig::allScenes)
		.property("splash screen GUI", &BootConfig::splashGui);
}


// static
std::string const BootConfig::s_FileName("config/boot_config.json");


//-----------------------------
// BootConfig::LoadFromPackage
//
// Configuration for engine bootup that cannot be modified by the user
//
void BootConfig::LoadFromPackage(BootConfig& cfg, core::I_Package* const pkg)
{
	std::vector<uint8> rawData;
	if (!pkg->GetEntryData(core::HashString(s_FileName.c_str()), rawData))
	{
		ET_ERROR("Failed to load boot config from package!");
		return;
	}

	// convert that data to a string and deserialize it as json
	core::JsonDeserializer deserializer;
	if (!deserializer.DeserializeFromData(rawData, cfg))
	{
		ET_ERROR("Failed to deserialize boot config at '%s'!", s_FileName);
	}
}


} // namespace fw
} // namespace et

