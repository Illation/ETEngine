#pragma once


namespace et {
namespace fw {


//------------------
// BootConfig
//
// Configuration for engine bootup that cannot be modified by the user
//
struct BootConfig final
{
	core::HashString startScene;
	std::vector<core::HashString> allScenes;
};


} // namespace fw
} // namespace et

