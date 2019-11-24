#pragma once
#include <rttr/type>

#include <EtCore/Helper/Singleton.h>

#include <EtRendering/GlobalRenderingSystems/GraphicsSettings.h>


//---------------------------------
// Config
//
// Config data for this project
//
class Config : public Singleton<Config>
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

		RTTR_ENABLE()
	};

	//---------------------------------
	// Config::Settings
	//
	// Configuration settings
	//
	class Settings
	{
	public:
		//---------------------------------
		// Config::Settings::Window
		//
		// Configuration for window settings
		//
		struct Window
		{
			Window() = default;

			// accessors
			//-----------
			ivec2 GetSize() const;

			// Settings loaded from JSON
			//---------------------------
			std::string Title = "ETEngine";
			bool Fullscreen = false;
			std::vector<ivec2> Resolutions;
			size_t FullscreenRes;
			size_t WindowedRes;
		};

		render::GraphicsSettings m_Graphics;
		Window m_Window;
		std::string m_StartScene;
		std::string m_ScreenshotDir;

		RTTR_ENABLE()
	};

private:
	friend class Singleton<Config>;


	// Default constructor and destructor
	//-----------------
	Config() = default;
	virtual ~Config();


	// Public interface
	//-----------------
public:
	std::string const& GetUserDirPath() const { return m_UserDir.m_UserDirPath; }

	Settings::Window const& GetWindow() const { return m_Settings.m_Window; }
	Settings::Window & GetWindow() { return m_Settings.m_Window; }

	std::string const& GetStartScene() const { return m_Settings.m_StartScene; }
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

