#include "stdafx.h"
#include "SceneEditor.h"

#include <rttr/registration>

#include <EtCore/Util/InputManager.h>

#include <EtFramework/SceneGraph/UnifiedScene.h>
#include <EtFramework/Physics/PhysicsManager.h>
#include <EtFramework/Audio/AudioManager.h>

#include <EtEditor/Rendering/EditorCamera.h>
#include <EtEditor/Util/GtkUtil.h>
#include <EtEditor/Util/EditorConfig.h>
#include <EtEditor/Tools/SceneViewport.h>


namespace et {
namespace edit {


//=======================
// Editor Meta Component
//=======================

RTTR_REGISTRATION
{
	using namespace rttr;

	registration::class_<EditorMetaComponent>("editor meta component")
		.property("name", &EditorMetaComponent::name);
}

ECS_REGISTER_COMPONENT(EditorMetaComponent);


//==========================
// Scene Editor
//==========================


// statics
std::string const SceneEditor::s_EditorName("Scene Editor");
std::string const SceneEditor::s_LayoutName("scene_editor");
std::vector<E_EditorTool> const SceneEditor::s_SupportedTools = {
	E_EditorTool::Outliner,
	E_EditorTool::ResourceBrowser,
	E_EditorTool::SceneViewport
	};


//--------------------------------
// SceneEditor::RegisterEcsEvents
//
// Any editor specific ecs events and systems should be added here
//
void SceneEditor::RegisterEcsEvents()
{
	fw::EcsController& ecs = fw::UnifiedScene::Instance().GetEcs();

	ecs.RegisterOnEntityAdded(fw::T_EntityEventFn([](fw::EcsController& ecs, fw::T_EntityId const entity) -> void
		{
			EditorMetaComponent comp;
			comp.name = FS("Entity_%u", entity);

			ecs.AddComponents(entity, comp);
		}));

	ecs.RegisterSystem<EditorCameraSystem>();
}

//---------------------------
// SceneEditor::d-tor
//
SceneEditor::~SceneEditor()
{
	fw::PhysicsManager::DestroyInstance();
	fw::AudioManager::DestroyInstance();
}

//---------------------------
// SceneEditor::InitInternal
//
// create the tools and attach them to the parent frame
//
void SceneEditor::InitInternal()
{
	m_IsShown = true;
	for (I_SceneEditorListener* const listener : m_Listeners)
	{
		listener->OnShown();
	}

	fw::AudioManager::GetInstance()->Initialize();
	fw::PhysicsManager::GetInstance()->Initialize();

	m_SceneSelection.SetScene();
	for (I_SceneEditorListener* const listener : m_Listeners)
	{
		listener->OnSceneSet();
	}

	fw::UnifiedScene::Instance().LoadScene(EditorConfig::GetInstance()->GetStartScene());
}

//----------------------------------------------------
// SceneEditor::OnKeyEvent
//
bool SceneEditor::OnKeyEvent(bool const pressed, GdkEventKey* const evnt)
{
	if (m_NavigatingViewport != nullptr)
	{
		return m_NavigatingViewport->OnKeyEvent(pressed, evnt);
	}

	return false;
}

//----------------------------------------------------
// SceneEditor::OnTick
//
void SceneEditor::OnTick()
{
	m_SceneSelection.UpdateOutlines();

	for (I_SceneEditorListener* const listener : m_Listeners)
	{
		listener->OnEditorTick();
	}
}

//----------------------------------------------------
// SceneEditor::RegisterListener
//
void SceneEditor::RegisterListener(I_SceneEditorListener* const listener)
{
	ET_ASSERT(std::find(m_Listeners.cbegin(), m_Listeners.cend(), listener) == m_Listeners.cend(), "Listener already registered!");

	m_Listeners.emplace_back(listener);
}

//----------------------------------------------------
// SceneEditor::UnregisterListener
//
void SceneEditor::UnregisterListener(I_SceneEditorListener const* const listener)
{
	// try finding the listener
	auto listenerIt = std::find(m_Listeners.begin(), m_Listeners.end(), listener);

	// it should have been found
	if (listenerIt == m_Listeners.cend())
	{
		LOG("SceneEditor::UnregisterListener > Listener not found", core::LogLevel::Warning);
		return;
	}

	// swap and remove - the order of the listener list doesn't matter
	if (m_Listeners.size() > 1u)
	{
		std::iter_swap(listenerIt, std::prev(m_Listeners.end()));
		m_Listeners.pop_back();
	}
	else
	{
		m_Listeners.clear();
	}
}


} // namespace edit
} // namespace et
