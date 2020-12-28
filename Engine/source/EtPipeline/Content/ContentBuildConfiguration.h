#pragma once


namespace et {
namespace pl {


//------------------------------
// BuilConfiguration
//
// Provides info for how to build assets for runtime
//
struct BuildConfiguration
{
	// definitions
	//-------------
	enum class E_Configuration : uint8
	{
		Debug,
		Develop,
		Shipping
	};

	enum class E_Architecture : uint8
	{
		x32,
		x64
	};

	enum class E_Platform : uint8
	{
		Windows,
		Linux
	};

	// construct destruct
	//--------------------
	BuildConfiguration();
	BuildConfiguration(E_Configuration const config, E_Architecture const arch, E_Platform const platform);

	// Data
	///////

	E_Configuration m_Configuration;
	E_Architecture m_Architecture;
	E_Platform m_Platform;
};


} // namespace pl
} // namespace et

