#pragma once
#include <rttr/type>

#include <EtCore/Util/Singleton.h>
#include <EtCore/Util/WindowSettings.h>

#include <EtRendering/GlobalRenderingSystems/GraphicsSettings.h>


namespace et {
namespace fw {


//---------------------------------
// Config
//
// Config data for this project
//
class Config : public core::Singleton<Config>
{
	// Definitions
	//-----------------
public:

	static constexpr char s_PointerPath[] = "userDirPointer.json";
	static constexpr char s_ConfigFileRelativePath[] = "./config.json";

	//---------------------------------
	// Config::UserDirPointer
	//
	// figure out where our user data lives
	//
	struct UserDirPointer
	{
		std::string m_UserDirPath;
	};

	//---------------------------------
	// Config::Settings
	//
	// Configuration settings
	//
	class Settings
	{
	public:
		render::GraphicsSettings m_Graphics;
		core::WindowSettings m_Window;
		std::string m_ScreenshotDir;
	};

private:
	friend class core::Singleton<Config>;


	// Default constructor and destructor
	//-----------------
	Config() = default;
	virtual ~Config();


	// Public interface
	//-----------------
public:
	std::string const& GetUserDirPath() const { return m_UserDir.m_UserDirPath; }

	core::WindowSettings const& GetWindow() const { return m_Settings.m_Window; }
	core::WindowSettings & GetWindow() { return m_Settings.m_Window; }

	std::string const& GetScreenshotDir() const { return m_Settings.m_ScreenshotDir; }

	// initialization
	void Initialize();
	void InitRenderConfig();
	void Save();

	// DATA
	///////
private:

	UserDirPointer m_UserDir;

	Settings m_Settings;
	bool m_HasRenderRef = false;
};


} // namespace fw
} // namespace et
