#pragma once


namespace et {
namespace fw {


//------------------
// BootConfig
//
// Configuration for engine loadup that cannot be modified by the user
//
struct BootConfig final
{
	// these should be replaced by hashnames
	std::string startScene;
	std::vector<std::string> allScenes;
};


} // namespace fw
} // namespace et

