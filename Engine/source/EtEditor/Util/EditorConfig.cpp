#include <EtFramework/stdafx.h>
#include "EditorConfig.h"

#include "ResourceChooserDialog.h"

#include <EtCore/Reflection/Serialization.h>
#include <EtCore/Reflection/Registration.h>


namespace et {
namespace edit {


//======================
// Editor Config
//======================


// reflection
RTTR_REGISTRATION
{
	using namespace rttr;

	registration::class_<EditorConfig::DirPointers>("dir pointers")
		.property("project dir path", &EditorConfig::DirPointers::m_ProjectPath)
		.property("engine dir path", &EditorConfig::DirPointers::m_EnginePath);
}


//---------------------------------
// EditorConfig::Initialize
//
// Load from JSON and generate derived data
//
void EditorConfig::Initialize()
{
	// try deserializing the user directory path
	if (!(core::serialization::DeserializeFromFile(s_PointerPath, m_DirPointers)))
	{
		LOG("EditorConfig::Initialize > unable to deserialize directory pointers!", core::LogLevel::Error);
	}

	m_DirPointers.m_EditorUserDir = m_DirPointers.m_ProjectPath + std::string("/user_data/editor/");
}

//---------------------------------
// EditorConfig::QueryStartScene
//
// Let the user decide what scene to boot into
//
void EditorConfig::QueryStartScene()
{
	ResourceChooserDialog* resourceDialog = ResourceChooserDialog::create();

	resourceDialog->signal_hide().connect([resourceDialog]() -> void { delete resourceDialog; });

	resourceDialog->run();

	m_StartScene = core::HashString("PlanetScene.json");
}


} // namespace edit
} // namespace et
