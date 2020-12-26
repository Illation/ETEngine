#include "stdafx.h"
#include "EditorApp.h"

#pragma warning( push )
#pragma warning( disable : 4244 ) // glib warnings
#include <glibmm/main.h>
#include <giomm/applicationcommandline.h>
#pragma warning( pop )

#include <glib/gerror.h>
#include <glibmm/ustring.h>
#include <gio/gresource.h>
#include <giomm/resource.h>
#include <glibmm/vectorutils.h>

#include <EtBuild/EngineVersion.h>

#include <EtCore/Util/Commands.h>
#include <EtCore/Util/InputManager.h>
#include <EtCore/UpdateCycle/TickManager.h>

#include <EtRendering/GraphicsContext/ContextHolder.h>
#include <EtRendering/GlobalRenderingSystems/GlobalRenderingSystems.h>

#include <EtFramework/SceneGraph/UnifiedScene.h>

#include <EtPipeline/Content/FileResourceManager.h>

#include <EtEditor/Layout/EditorWindow.h>
#include <EtEditor/Util/SettingsDialog.h>
#include <EtEditor/Util/EditorConfig.h>
#include <EtEditor/SceneEditor/SceneEditor.h>


namespace et {
namespace edit {


//====================
// Editor Application 
//====================


//---------------------------------
// EditorApp::c-tor
//
EditorApp::EditorApp()
	: Gtk::Application("com.leah-lindner.editor", Gio::APPLICATION_HANDLES_OPEN)
	, m_CmdArguments(CommandlineArguments())
{ 
	RegisterAsTriggerer();
	
	core::Logger::Initialize();//Init logger first because all output depends on it from the start

	LOG(FS("E.T.Editor"));
	LOG(FS("//////////"));
	LOG("");
	LOG(FS(" - version: %s", et::build::Version::s_Name.c_str()));
	LOG("");

	InitializeUtilities();

	// Allow updating every frame in a gameloop style - called as quickly as possible
	Glib::signal_idle().connect(sigc::mem_fun(*this, &EditorApp::OnTick));

	// Initialize commandline arguments for later
	auto onCommandLine = [this](const Glib::RefPtr<Gio::ApplicationCommandLine>& command_line) -> int32
	{
		int argc;
		char **argv = command_line->get_arguments(argc);
		m_CmdArguments = CommandlineArguments(argc, argv);
		return 0;
	};
	signal_command_line().connect(onCommandLine, false);

	// when we press close the last window
	signal_shutdown().connect(
		[this]()
	{
		core::InputManager::GetInstance()->Quit();
		OnActionQuit();
	});
}

//---------------------------------
// EditorApp::d-tor
//
EditorApp::~EditorApp()
{
	render::RenderingSystems::RemoveReference();

	core::PerformanceInfo::DestroyInstance();
	core::InputManager::DestroyInstance();

	EditorConfig::DestroyInstance();

	core::ResourceManager::DestroyInstance();

	core::Logger::Release();
	core::TickManager::GetInstance()->DestroyInstance();
}

//---------------------------------
// EditorApp::create
//
// creation factory
//
Glib::RefPtr<EditorApp> EditorApp::create()
{
	return Glib::RefPtr<EditorApp>(new EditorApp());
}

//---------------------------------
// EditorApp::CreateMainWindow
//
// Create the main window for the synth application
//
EditorAppWindow* EditorApp::CreateMainWindow()
{
	EditorAppWindow* appwindow = EditorAppWindow::create(this);

	// Make sure that the application runs for as long this window is still open.
	add_window(*appwindow);

	// Gtk::Application::add_window() connects a signal handler to the window's
	// signal_hide(). That handler removes the window from the application.
	// If it's the last window to be removed, the application stops running.
	// Gtk::Window::set_application() does not connect a signal handler, but is
	// otherwise equivalent to Gtk::Application::add_window().

	// Delete the window when it is hidden.
	appwindow->signal_hide().connect(sigc::bind<Gtk::Window*>(sigc::mem_fun(*this, &EditorApp::OnHideWindow), appwindow));

	return appwindow;
}

//---------------------------------
// EditorApp::OnHideWindow
//
// Hiding a window will make us delete it
//
void EditorApp::OnHideWindow(Gtk::Window* window)
{
	delete window;
}

//---------------------------------
// EditorApp::on_startup
//
// When the Gtk::Application gets activated we create a window and present it
//
void EditorApp::on_startup()
{
	// Call the base class's implementation.
	Gtk::Application::on_startup();

	// Add actions and keyboard accelerators for the application menu.
	add_action("preferences", sigc::mem_fun(*this, &EditorApp::OnActionPreferences));
	add_action("save_layout", sigc::mem_fun(*this, &EditorApp::OnSaveEditorLayout));
	add_action("quit", sigc::mem_fun(*this, &EditorApp::OnActionQuit));
	set_accel_for_action("app.quit", "<Ctrl>Q");

	Glib::RefPtr<Gtk::Builder> refBuilder = Gtk::Builder::create();
	try
	{
		refBuilder->add_from_resource("/com/leah-lindner/editor/ui/app_menu.ui");
	}
	catch (const Glib::Error& ex)
	{
		LOG("EditorApp::on_startup > " + std::string(ex.what()), core::LogLevel::Error);
		return;
	}
	catch (std::exception const& ex)
	{
		LOG("EditorApp::on_startup > " + std::string(ex.what()), core::LogLevel::Error);
		return;
	}

	auto appMenu = Glib::RefPtr<Gio::MenuModel>::cast_dynamic(refBuilder->get_object("appmenu"));
	if (appMenu)
	{
		set_app_menu(appMenu);
	}
	else
	{
		LOG("EditorApp::on_startup > No 'appmenu' object in app_menu.ui", core::LogLevel::Error);
	}
}

//---------------------------------
// EditorApp::on_activate
//
// When the Gtk::Application gets activated we create a window and present it
//
void EditorApp::on_activate()
{
	// The application has been started, so let's show a window.
	try
	{
		m_AppWindow = CreateMainWindow();
		m_AppWindow->present(); 

		// window is realized now create rendering context
		m_RenderWindow.SetSourceWindow(m_AppWindow);
		render::ContextHolder::Instance().CreateMainRenderContext(&m_RenderWindow);

		render::RenderingSystems::AddReference();

		// from now on it should be safe to call graphics API functions
		m_AppWindow->AddEditor(new SceneEditor());
	}
	// If create_appwindow() throws an exception (perhaps from Gtk::Builder),
	// no window has been created, no window has been added to the application,
	// and therefore the application will stop running.
	catch (Glib::Error const& ex)
	{
		LOG("EditorApp::on_activate > " + std::string(ex.what().c_str()), core::LogLevel::Error);
	}
	catch (std::exception const& ex)
	{
		LOG("EditorApp::on_activate > " + std::string(ex.what()), core::LogLevel::Error);
	}
}

//---------------------------------
// EditorApp::InitializeUtilities
//
// Initialize all the other cool stuff
//
void EditorApp::InitializeUtilities()
{
	EditorConfig* const edConfig = EditorConfig::GetInstance();
	edConfig->Initialize();

	fw::UnifiedScene::Instance().GetEventDispatcher().Register(fw::E_SceneEvent::RegisterSystems,
		fw::T_SceneEventCallback([this](fw::T_SceneEventFlags const flags, fw::SceneEventData const* const eventData)
		{
			UNUSED(flags);
			UNUSED(eventData);

			SceneEditor::RegisterEcsEvents();
		}));
	fw::UnifiedScene::Instance().Init();

	core::ResourceManager::SetInstance(new pl::FileResourceManager(edConfig->GetProjectPath(), edConfig->GetEnginePath()));

	edConfig->QueryStartScene();

	core::InputManager::GetInstance();

	core::PerformanceInfo::GetInstance();
}

//---------------------------------
// EditorApp::OnTick
//
// What do we want to do every cycle?
//
bool EditorApp::OnTick()
{
	if (!(core::InputManager::GetInstance()->IsRunning()))
	{
		return false;
	}

	TriggerTick(); // try triggering a tick in case this is not being handled by realtime triggerers

	return true; // we want to keep the callback alive
}

//---------------------------------
// EditorApp::OnActionPreferences
//
// On clicking the preferences button in the menu we show the SettingsDialog
//
void EditorApp::OnActionPreferences()
{
	try
	{
		SettingsDialog* prefsDialog = SettingsDialog::create(*get_active_window());
		prefsDialog->present();

		// Delete the dialog when it is hidden.
		prefsDialog->signal_hide().connect(sigc::bind<Gtk::Window*>(sigc::mem_fun(*this, &EditorApp::OnHideWindow), prefsDialog));
	}
	catch (const Glib::Error& ex)
	{
		LOG("EditorApp::OnActionPreferences > " + std::string(ex.what()), core::LogLevel::Error);
	}
	catch (const std::exception& ex)
	{
		LOG("EditorApp::OnActionPreferences > " + std::string(ex.what()), core::LogLevel::Error);
	}
}

//---------------------------------
// EditorApp::OnActionPreferences
//
// On clicking the preferences button in the menu we show the SettingsDialog
//
void EditorApp::OnSaveEditorLayout()
{
	ET_ASSERT(m_AppWindow != nullptr);

	m_AppWindow->SaveLayout();
}

//---------------------------------
// EditorApp::OnActionQuit
//
// On clicking the quit button in the menu
//
void EditorApp::OnActionQuit()
{
	// Gio::Application::quit() will make Gio::Application::run() return,
	// but it's a crude way of ending the program. The window is not removed
	// from the application. Neither the window's nor the application's
	// destructors will be called, because there will be remaining reference
	// counts in both of them. If we want the destructors to be called, we
	// must remove the window from the application. One way of doing this
	// is to hide the window. See comment in CreateMainWindow().
	auto windows = get_windows();
	for (auto window : windows)
	{
		window->hide();
	}
}


} // namespace edit
} // namespace et
