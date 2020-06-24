#pragma once
#include <string>
#include <vector>

#include <rttr/type>

#include <EtCore/Util/Singleton.h>


namespace et {
namespace edit {


class EditorApp;


//---------------------------------
// EditorConfig
//
// Config data for this projects editor
//
class EditorConfig : public core::Singleton<EditorConfig>
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
	friend class core::Singleton<EditorConfig>;


	// Default constructor and destructor
	//-----------------
	EditorConfig() = default;
	virtual ~EditorConfig() = default;


	// accessors
	//-----------
public:
	std::string const& GetProjectPath() const { return m_DirPointers.m_ProjectPath; }
	std::string const& GetEnginePath() const { return m_DirPointers.m_EnginePath; }
	std::string const& GetEditorUserDir() const { return m_DirPointers.m_EditorUserDir; }
	core::HashString GetStartScene() const { return m_StartScene; }

	// functionality
	//---------------
	void Initialize();
	void QueryStartScene();

	// DATA
	///////
private:

	DirPointers m_DirPointers;

	core::HashString m_StartScene;
};


} // namespace edit
} // namespace et
