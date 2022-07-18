#include <EtEditor/stdafx.h>
#include "EditorConfig.h"

#include "ResourceChooserDialog.h"

#include <EtCore/Reflection/Serialization.h>
#include <EtCore/Reflection/Registration.h>

#include <EtPipeline/Content/EditorAsset.h>
#include <EtPipeline/Assets/EditableSceneDescriptor.h>


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
		ET_LOG_E(ET_CTX_EDITOR, "EditorConfig::Initialize > unable to deserialize directory pointers!");
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
	ResourceChooserDialog* resourceDialog = ResourceChooserDialog::create(std::vector<rttr::type>(
		{
			rttr::type::get<pl::EditableSceneDescriptorAsset>()
		}));
	resourceDialog->set_title("Please select a Scene...");

	resourceDialog->signal_hide().connect([resourceDialog]() -> void { delete resourceDialog; });

	int32 const response = resourceDialog->run();
	if (response == Gtk::ResponseType::RESPONSE_ACCEPT)
	{
		ET_ASSERT(!(resourceDialog->GetSelectedAssets().empty()));

		pl::EditorAssetBase* const selectedAsset = resourceDialog->GetSelectedAssets()[0];
		ET_ASSERT(selectedAsset->GetType() == rttr::type::get<fw::SceneDescriptor>());
		
		m_StartScene = selectedAsset->GetId();
	}
	else
	{
		m_StartScene.Reset();
	}

	resourceDialog->hide();
}


} // namespace edit
} // namespace et
