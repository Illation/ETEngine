#include "stdafx.h"
#include "BootConfig.h"

#include <EtCore/Reflection/Registration.h>


namespace et {
namespace rt {


//=============
// Boot Config
//=============


// reflection
RTTR_REGISTRATION
{
	rttr::registration::class_<BootConfig>("boot config")
		.property("start scene", &BootConfig::startScene)
		.property("all scenes", &BootConfig::allScenes);
}


// static
et::core::HashString const BootConfig::s_FileName("BootConfig.json");


//-----------------------------
// BootConfig::LoadFromPackage
//
// Configuration for engine bootup that cannot be modified by the user
//
void BootConfig::LoadFromPackage(BootConfig& cfg, core::I_Package* const pkg)
{

}


} // namespace rt
} // namespace et

