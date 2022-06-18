#pragma once


// fwd
namespace et { namespace core {
	class I_Package;
} }


namespace et {
namespace fw {


//------------------
// BootConfig
//
// Configuration for engine bootup that cannot be modified by the user
//
struct BootConfig final
{
	static std::string const s_FileName; // string for file reading
	static void LoadFromPackage(BootConfig& cfg, core::I_Package* const pkg);

	core::HashString startScene;
	std::vector<core::HashString> allScenes;
	core::HashString splashGui;
};


} // namespace fw
} // namespace et

