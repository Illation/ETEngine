#pragma once
#include <string>
#include <vector>

#include <rttr/type>

#include <EtCore/Helper/Singleton.h>


//---------------------------------
// EditorConfig
//
// Config data for this projects editor
//
class EditorConfig : public Singleton<EditorConfig>
{
	// Definitions
	//-----------------
public:

	static constexpr char s_PointerPath[] = "../config/dirPointers.json";

	//---------------------------------
	// EditorConfig::DirPointers
	//
	// figure out where our user data lives
	//
	struct DirPointers
	{
		std::string m_ProjectPath;
		std::string m_EnginePath;

		// derived
		std::string m_EditorUserDir;

		RTTR_ENABLE()
	};

private:
	friend class Singleton<EditorConfig>;


	// Default constructor and destructor
	//-----------------
	EditorConfig() = default;
	virtual ~EditorConfig() = default;


	// Public interface
	//-----------------
public:
	std::string const& GetProjectPath() const { return m_DirPointers.m_ProjectPath; }
	std::string const& GetEnginePath() const { return m_DirPointers.m_EnginePath; }
	std::string const& GetEditorUserDir() const { return m_DirPointers.m_EditorUserDir; }

	// initialization
	void Initialize();

	// DATA
	///////
private:

	DirPointers m_DirPointers;
};

