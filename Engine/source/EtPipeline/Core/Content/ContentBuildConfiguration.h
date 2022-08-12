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
		Invalid,

		Debug,
		Develop,
		Shipping
	};

	enum class E_Architecture : uint8
	{
		Invalid,

		x32,
		x64
	};

	enum class E_Platform : uint8
	{
		Invalid,

		Windows,
		Linux
	};

	enum class E_GraphicsBackend : uint8 
	{
		OpenGL
	};

	// construct destruct
	//--------------------
	BuildConfiguration();
	BuildConfiguration(E_Configuration const config, E_Architecture const arch, E_Platform const platform);

	// Data
	///////

	E_Configuration m_Configuration = E_Configuration::Invalid;
	E_Architecture m_Architecture = E_Architecture::Invalid;
	E_Platform m_Platform = E_Platform::Invalid;
	E_GraphicsBackend m_GraphicsBackend = E_GraphicsBackend::OpenGL;
};


} // namespace pl
} // namespace et

