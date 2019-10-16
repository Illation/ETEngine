#include "stdafx.h"
#include "SceneEditor.h"

#include "EditorScene.h"

#include <gtkmm/paned.h>
#include <gtkmm/frame.h>
#include <gtkmm/box.h>

#include <EtCore/Helper/InputManager.h>

#include <Engine/SceneGraph/SceneManager.h>
#include <Engine/SceneRendering/SceneRenderer.h>
#include <Engine/Physics/PhysicsManager.h>
#include <Engine/Audio/AudioManager.h>

#include <EtEditor/UI/GtkUtil.h>


//==========================
// Scene Editor
//==========================


// statics
std::string const SceneEditor::s_EditorName("Scene Editor");


//---------------------------
// SceneEditor::d-tor
//
SceneEditor::~SceneEditor()
{
	SceneManager::DestroyInstance();
	PhysicsManager::DestroyInstance();
	AudioManager::DestroyInstance();
}

//---------------------------
// SceneEditor::Init
//
// create the tools and attach them to the parent frame
//
void SceneEditor::Init(Gtk::Frame* const parent)
{
	// get the toplevel element
	Gtk::Paned* const paned = Gtk::make_managed<Gtk::Paned>(Gtk::ORIENTATION_HORIZONTAL);
	parent->add(*paned);

	Gtk::Paned* const paned2 = Gtk::make_managed<Gtk::Paned>(Gtk::ORIENTATION_VERTICAL);
	CreateInnerFrame(paned, false)->add(*paned2);

	CreateTool(E_EditorTool::SceneViewport, CreateInnerFrame(paned, true));
	CreateTool(E_EditorTool::Outliner, CreateInnerFrame(paned2, true));
	//CreateTool(E_EditorTool::SceneViewport, CreateInnerFrame(paned2, false));
	paned->set_position(1000);
	paned2->set_position(500);

	paned->show_all_children();
	for (I_SceneEditorListener* const listener : m_Listeners)
	{
		listener->OnShown();
	}

	AudioManager::GetInstance()->Initialize();
	PhysicsManager::GetInstance()->Initialize();

	SceneManager::GetInstance()->AddGameScene(new EditorScene());
	SceneManager::GetInstance()->SetActiveGameScene("EditorScene");
	m_SceneSelection.SetScene(SceneManager::GetInstance()->GetNewActiveScene());
	for (I_SceneEditorListener* const listener : m_Listeners)
	{
		listener->OnSceneSet();
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
		LOG("SceneEditor::UnregisterListener > Listener not found", LogLevel::Warning);
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

