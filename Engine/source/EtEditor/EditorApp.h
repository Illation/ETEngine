#pragma once

#include "SceneSelection.h"

#include <EtCore/UpdateCycle/DefaultTickTriggerer.h>

#include <list>
#include <memory>

#include <gtkmm/application.h>
#include <glibmm/refptr.h>
#include <glibmm/ustring.h>

// forward declarations
class EditorAppWindow;

//---------------------------------
// CommandlineArguments
//
// Store commandline arguments here so we can look them up later, might do more complex things with this at a later point
//
struct CommandlineArguments
{
	CommandlineArguments() = default;
	CommandlineArguments(int argc, char *argv[]) : argumentCount(argc), argumentValues(argv) {}

	int32 argumentCount = 0;
	char** argumentValues = nullptr;
};

//---------------------------------
// EditorApp
//
// Application for the engine editor
//
class EditorApp final : public Gtk::Application, public I_DefaultTickTriggerer
{
protected:
	EditorApp();
	virtual ~EditorApp();

public:
	static Glib::RefPtr<EditorApp> create();

private:
	EditorAppWindow* CreateMainWindow();
	void OnHideWindow(Gtk::Window* window);

protected:
	// Override default gtkmm application signal handlers:
	void on_startup() override;
	void on_activate() override;

private:
	void InitializeUtilities();

public:
	SceneSelection& GetSceneSelection() { return m_SceneSelection; }

	// Runtime
private:
	bool OnTick();
	void OnActionPreferences();
	void OnActionQuit();

private:
	// Data
	////////
	CommandlineArguments m_CmdArguments;
};


class SceneEditor
{
public:
	std::unique_ptr<Viewport> CreateSceneViewport();
	std::unique_ptr<Outliner> CreateOutliner();

	static E_MouseButton GetButtonFromGtk(uint32 const buttonCode);
	static E_KbdKey GetKeyFromGtk(uint32 const keyCode);

private:

	// Data
	///////

	SceneSelection m_SceneSelection;

	bool m_IsNavigating = false;

	std::unique_ptr<Viewport> m_SceneViewport;
	std::unique_ptr<Outliner> m_Outliner;
};
