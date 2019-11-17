#pragma once
#include <rttr/type>

#include <EtCore/Helper/Singleton.h>


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
		// Config::Settings::Graphics
		//
		// Configuration for our rt audio output stream
		//
		struct Graphics
		{
			bool UseFXAA = true;

			//Shadow Quality
			int32 NumCascades = 3;
			float CSMDrawDistance = 200.f;
			int32 NumPCFSamples = 3;

			int32 PbrBrdfLutSize = 512;

			float TextureScaleFactor = 1.f;

			//Bloom Quality
			int32 NumBlurPasses = 5;
		};

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

		Graphics m_Graphics;
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
	virtual ~Config() = default;


	// Public interface
	//-----------------
public:
	std::string const& GetUserDirPath() const { return m_UserDir.m_UserDirPath; }

	Settings::Graphics const& GetGraphics() const { return m_Settings.m_Graphics; }
	Settings::Graphics & GetGraphics() { return m_Settings.m_Graphics; }

	Settings::Window const& GetWindow() const { return m_Settings.m_Window; }
	Settings::Window & GetWindow() { return m_Settings.m_Window; }

	std::string const& GetStartScene() const { return m_Settings.m_StartScene; }
	std::string const& GetScreenshotDir() const { return m_Settings.m_ScreenshotDir; }

	// initialization
	void Initialize();
	void Save();

	// DATA
	///////
private:

	UserDirPointer m_UserDir;

	Settings m_Settings;
};

