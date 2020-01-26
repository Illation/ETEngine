#pragma once


// fwd
namespace et { namespace core {
	class I_Package;
} }


namespace et {
namespace rt {


//------------------
// BootConfig
//
// Configuration for engine bootup that cannot be modified by the user
//
struct BootConfig final
{
	static core::HashString const s_FileName;
	static void LoadFromPackage(BootConfig& cfg, core::I_Package* const pkg);

	core::HashString startScene;
	std::vector<core::HashString> allScenes;
};


} // namespace rt
} // namespace et

