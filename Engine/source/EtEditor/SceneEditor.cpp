#include "stdafx.h"
#include "SceneEditor.h"

#include "EditorScene.h"

#include <EtCore/Helper/InputManager.h>

#include <Engine/SceneGraph/SceneManager.h>
#include <Engine/Physics/PhysicsManager.h>
#include <Engine/Audio/AudioManager.h>

#include <gtkmm/paned.h>
#include <gtkmm/box.h>

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

	m_SceneViewport = std::move(CreateSceneViewport());
	m_SceneViewport->Init(this, CreateInnerFrame(paned, true));
	m_Outliner = std::move(CreateOutliner());
	m_Outliner->Init(this, CreateInnerFrame(paned, false));

	paned->set_position(1000);
	paned->show_all_children();

	m_SceneViewport->ShowSplashScreen();

	AudioManager::GetInstance()->Initialize();
	PhysicsManager::GetInstance()->Initialize();

	SceneManager::GetInstance()->AddGameScene(new EditorScene());
	SceneManager::GetInstance()->SetActiveGameScene("EditorScene");
	m_SceneSelection.SetScene(SceneManager::GetInstance()->GetNewActiveScene());

	m_SceneViewport->InitRenderingSystems();
}

//---------------------------------
// SceneEditor::CreateSceneViewport
//
// Create a viewport with an openGL area in it
//
std::unique_ptr<SceneViewport> SceneEditor::CreateSceneViewport()
{
	return std::make_unique<SceneViewport>();
}

//---------------------------------
// SceneEditor::CreateOutliner
//
std::unique_ptr<Outliner> SceneEditor::CreateOutliner()
{
	return std::make_unique<Outliner>();
}

//---------------------------------
// SceneEditor::CreateInnerFrame
//
Gtk::Frame* SceneEditor::CreateInnerFrame(Gtk::Paned* const split, bool const isFirst)
{
	Gtk::Frame* childFrame = Gtk::make_managed<Gtk::Frame>();
	childFrame->set_shadow_type(Gtk::SHADOW_ETCHED_IN);

	if (isFirst)
	{
		split->add1(*childFrame);
	}
	else
	{
		split->add2(*childFrame);
	}

	return childFrame;
}

