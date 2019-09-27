#include "stdafx.h"
#include "EditorWindow.h"

#include <EtEditor/EditorApp.h>
#include <EtEditor/Rendering/GtkRenderArea.h>

#include <EtCore/Helper/InputManager.h>

#include <gtkmm/object.h>
#include <gtkmm/scrolledwindow.h>
#include <gtkmm/textview.h>
#include <gtkmm/settings.h>
#include <gtkmm/glarea.h>

#include <Engine/GraphicsHelper/SceneRenderer.h>


namespace Gtk
{
	template<class T, class... T_Args>
	auto make_managed(T_Args&&... args)
	{
		return manage(new T(std::forward<T_Args>(args)...));
	}
}


//===========================
// Editor Application Window
//===========================


//---------------------------------
// EditorAppWindow::c-tor
//
EditorAppWindow::EditorAppWindow(BaseObjectType* cobject, Glib::RefPtr<Gtk::Builder> const& refBuilder)
	: Gtk::ApplicationWindow(cobject)
	, m_RefBuilder(refBuilder)
	, m_Settings()
{
	m_Settings = Gio::Settings::create("com.leah-lindner.editor");

	// listen for keyboard input
	// on press
	auto keyPressedCallback = [](GdkEventKey* evnt) -> bool
	{
		InputManager::GetInstance()->OnKeyPressed(GetKeyFromGtk(evnt->keyval));
		return false;
	};
	signal_key_press_event().connect(keyPressedCallback, false);

	// on release
	auto keyReleasedCallback = [](GdkEventKey* evnt) -> bool
	{
		InputManager::GetInstance()->OnKeyReleased(GetKeyFromGtk(evnt->keyval));
		return false;
	};
	signal_key_release_event().connect(keyReleasedCallback, false);

	//show all the widgets
	show_all_children();

	// Display the application menu in the application, not in the desktop environment.
	Glib::RefPtr<Gtk::Settings> gtk_settings = Gtk::Settings::get_default();
	if (gtk_settings)
	{
		gtk_settings->property_gtk_shell_shows_app_menu() = false;
	}
	set_show_menubar(true);

	// Set the window icon.
	set_icon(Gdk::Pixbuf::create_from_resource("/com/leah-lindner/editor/ui/icons/menu.png"));
}

//static
//---------------------------------
// EditorAppWindow::create
//
// Create a editor app window from the generated source in window.ui
//
EditorAppWindow* EditorAppWindow::create(EditorApp *const editorApp)
{
	// Load the Builder file and instantiate its widgets.
	Glib::RefPtr<Gtk::Builder> refBuilder = Gtk::Builder::create_from_resource("/com/leah-lindner/editor/ui/editorWindow.ui");

	// get the toplevel element
	EditorAppWindow* window = nullptr;
	refBuilder->get_widget_derived("app_window", window);
	if (!window)
	{
		throw std::runtime_error("No 'app_window' object in window.ui");
	}

	window->SetEditorApp(editorApp);

	return window;
}

//---------------------------------
// EditorAppWindow::SetEditorApp
//
// Set the editor app so we can display components of the editor
//
void EditorAppWindow::SetEditorApp(EditorApp *const editorApp)
{
	m_EditorApp = editorApp;

	// create a new viewport
	m_SceneViewport = std::move(CreateSceneViewport());
}

//---------------------------------
// EditorAppWindow::Init
//
// After we have an openGl context and a resource manager we sho9uld show the splash screen
//
void EditorAppWindow::Init()
{
	SceneRenderer::GetInstance()->InitWithSplashScreen();
	m_SceneViewport->Redraw();
}

//---------------------------------
// EditorAppWindow::CreateSceneViewport
//
// Create a viewport with an openGL area in it
//
std::unique_ptr<Viewport> EditorAppWindow::CreateSceneViewport()
{
	// Find the GL Area widget that is responsible for rendering the scene
	Gtk::GLArea* glArea = nullptr;// Gtk::make_managed<Gtk::GLArea>();
	m_RefBuilder->get_widget("glSceneViewport", glArea);
	ET_ASSERT(glArea != nullptr, "No 'glSceneViewport' object in window.ui!");
	glArea->set_auto_render(true);

	// create a viewport from the area
	std::unique_ptr<Viewport> viewport = std::make_unique<Viewport>(new GtkRenderArea(glArea));

	// hook up events
	// mouse click
	glArea->add_events(Gdk::BUTTON_PRESS_MASK);
	auto mousePressedCallback = [](GdkEventButton* evnt) -> bool
	{
		InputManager::GetInstance()->OnMousePressed(GetButtonFromGtk(evnt->button));
		return false;
	};
	glArea->signal_button_press_event().connect(mousePressedCallback, false);

	// mouse release
	glArea->add_events(Gdk::BUTTON_RELEASE_MASK);
	auto mouseReleasedCallback = [](GdkEventButton* evnt) -> bool
	{
		InputManager::GetInstance()->OnMouseReleased(GetButtonFromGtk(evnt->button));
		return false;
	};
	glArea->signal_button_release_event().connect(mouseReleasedCallback, false);

	// mouse moved
	glArea->add_events(Gdk::POINTER_MOTION_MASK);
	auto mouseMotionCallback = [glArea](GdkEventMotion* evnt) -> bool
	{
		// get offset of widget to window position
		ivec2 pos = etm::vecCast<int32>(dvec2(evnt->x, evnt->y));
		pos = pos - ivec2(glArea->get_allocation().get_x(), glArea->get_allocation().get_y());

		InputManager::GetInstance()->OnMouseMoved(pos);
		return false;
	};
	glArea->signal_motion_notify_event().connect(mouseMotionCallback, false);

	// mouse scrolled
	glArea->add_events(Gdk::SMOOTH_SCROLL_MASK);
	glArea->add_events(Gdk::SCROLL_MASK);
	auto scrollCallback = [](GdkEventScroll* evnt) -> bool
	{
		dvec2 delta(evnt->delta_x, evnt->delta_y);
		if (etm::isZero(delta))
		{
			switch (evnt->direction)
			{
			case GdkScrollDirection::GDK_SCROLL_LEFT:
				delta.x = -1.0;
				break;

			case GdkScrollDirection::GDK_SCROLL_RIGHT:
				delta.x = 1.0;
				break;

			case GdkScrollDirection::GDK_SCROLL_UP:
				delta.y = 1.0;
				break;

			case GdkScrollDirection::GDK_SCROLL_DOWN:
				delta.y = -1.0;
				break;
			}
		}

		InputManager::GetInstance()->SetMouseWheelDelta(etm::vecCast<int32>(delta));
		return false;
	};
	glArea->signal_scroll_event().connect(scrollCallback, false);

	glArea->show();

	// create a scene renderer for the viewport
	viewport->SetRenderer(SceneRenderer::GetInstance());

	return std::move(viewport);
}

//---------------------------------
// EditorAppWindow::GetButtonFromGtk
//
// Convert GTK mouse button codes to the input managers buttons
//
E_MouseButton EditorAppWindow::GetButtonFromGtk(uint32 const buttonCode)
{
	switch (buttonCode)
	{
	case 1: return E_MouseButton::Left;
	case 2: return E_MouseButton::Center;
	case 3: return E_MouseButton::Right;

	default: return E_MouseButton::COUNT; // invalid button code
	}
}

//---------------------------------
// EditorAppWindow::GetButtonFromGtk
//
// Convert GTK key codes to the input managers keys
//
E_KbdKey EditorAppWindow::GetKeyFromGtk(uint32 const keyCode)
{
	switch (keyCode)
	{
	case GDK_KEY_space: return E_KbdKey::Space;
	case GDK_KEY_quoteright: return E_KbdKey::Apostrophe;
	case GDK_KEY_comma: return E_KbdKey::Comma;
	case GDK_KEY_minus: return E_KbdKey::Minus;
	case GDK_KEY_period: return E_KbdKey::Period;
	case GDK_KEY_slash: return E_KbdKey::Slash;

	case GDK_KEY_0: return E_KbdKey::Num_0;
	case GDK_KEY_1: return E_KbdKey::Num_1;
	case GDK_KEY_2: return E_KbdKey::Num_2;
	case GDK_KEY_3: return E_KbdKey::Num_3;
	case GDK_KEY_4: return E_KbdKey::Num_4;
	case GDK_KEY_5: return E_KbdKey::Num_5;
	case GDK_KEY_6: return E_KbdKey::Num_6;
	case GDK_KEY_7: return E_KbdKey::Num_7;
	case GDK_KEY_8: return E_KbdKey::Num_8;
	case GDK_KEY_9: return E_KbdKey::Num_9;

	case GDK_KEY_semicolon: return E_KbdKey::Semicolon;
	case GDK_KEY_equal: return E_KbdKey::Equal;

	case GDK_KEY_a: case GDK_KEY_A: return E_KbdKey::A;
	case GDK_KEY_b: case GDK_KEY_B: return E_KbdKey::B;
	case GDK_KEY_c: case GDK_KEY_C: return E_KbdKey::C;
	case GDK_KEY_d: case GDK_KEY_D: return E_KbdKey::D;
	case GDK_KEY_e: case GDK_KEY_E: return E_KbdKey::E;
	case GDK_KEY_f: case GDK_KEY_F: return E_KbdKey::F;
	case GDK_KEY_g: case GDK_KEY_G: return E_KbdKey::G;
	case GDK_KEY_h: case GDK_KEY_H: return E_KbdKey::H;
	case GDK_KEY_i: case GDK_KEY_I: return E_KbdKey::I;
	case GDK_KEY_j: case GDK_KEY_J: return E_KbdKey::J;
	case GDK_KEY_k: case GDK_KEY_K: return E_KbdKey::K;
	case GDK_KEY_l: case GDK_KEY_L: return E_KbdKey::L;
	case GDK_KEY_m: case GDK_KEY_M: return E_KbdKey::M;
	case GDK_KEY_n: case GDK_KEY_N: return E_KbdKey::N;
	case GDK_KEY_o: case GDK_KEY_O: return E_KbdKey::O;
	case GDK_KEY_p: case GDK_KEY_P: return E_KbdKey::P;
	case GDK_KEY_q: case GDK_KEY_Q: return E_KbdKey::Q;
	case GDK_KEY_r: case GDK_KEY_R: return E_KbdKey::R;
	case GDK_KEY_s: case GDK_KEY_S: return E_KbdKey::S;
	case GDK_KEY_t: case GDK_KEY_T: return E_KbdKey::T;
	case GDK_KEY_u: case GDK_KEY_U: return E_KbdKey::U;
	case GDK_KEY_v: case GDK_KEY_V: return E_KbdKey::V;
	case GDK_KEY_w: case GDK_KEY_W: return E_KbdKey::W;
	case GDK_KEY_x: case GDK_KEY_X: return E_KbdKey::X;
	case GDK_KEY_y: case GDK_KEY_Y: return E_KbdKey::Y;
	case GDK_KEY_z: case GDK_KEY_Z: return E_KbdKey::Z;

	case GDK_KEY_bracketleft: return E_KbdKey::LeftBracket;
	case GDK_KEY_backslash: return E_KbdKey::Backslash;
	case GDK_KEY_bracketright: return E_KbdKey::RightBracket;
	case GDK_KEY_quoteleft: return E_KbdKey::GraveAccent;
	case GDK_KEY_exclamdown: return E_KbdKey::World_1;
	case GDK_KEY_cent: return E_KbdKey::World_2;

	case GDK_KEY_Escape: return E_KbdKey::Escape;
	case GDK_KEY_Return: return E_KbdKey::Return;
	case GDK_KEY_Tab: return E_KbdKey::Tab;
	case GDK_KEY_BackSpace: return E_KbdKey::Backspace;
	case GDK_KEY_Insert: return E_KbdKey::Insert;
	case GDK_KEY_Delete: return E_KbdKey::Delete;
	case GDK_KEY_Right: return E_KbdKey::Right;
	case GDK_KEY_Left: return E_KbdKey::Left;
	case GDK_KEY_Down: return E_KbdKey::Down;
	case GDK_KEY_Up: return E_KbdKey::Up;
	case GDK_KEY_Page_Up: return E_KbdKey::PageUp;
	case GDK_KEY_Page_Down: return E_KbdKey::PageDown;
	case GDK_KEY_Home: return E_KbdKey::Home;
	case GDK_KEY_End: return E_KbdKey::End;
	case GDK_KEY_Caps_Lock: return E_KbdKey::CapsLock;
	case GDK_KEY_Scroll_Lock: return E_KbdKey::ScrollLock;
	case GDK_KEY_Num_Lock: return E_KbdKey::NumLock;
	case GDK_KEY_Print: return E_KbdKey::PrintScreen;
	case GDK_KEY_Pause: return E_KbdKey::Pause;

	case GDK_KEY_F1: return E_KbdKey::F1;
	case GDK_KEY_F2: return E_KbdKey::F2;
	case GDK_KEY_F3: return E_KbdKey::F3;
	case GDK_KEY_F4: return E_KbdKey::F4;
	case GDK_KEY_F5: return E_KbdKey::F5;
	case GDK_KEY_F6: return E_KbdKey::F6;
	case GDK_KEY_F7: return E_KbdKey::F7;
	case GDK_KEY_F8: return E_KbdKey::F8;
	case GDK_KEY_F9: return E_KbdKey::F9;
	case GDK_KEY_F10: return E_KbdKey::F10;
	case GDK_KEY_F11: return E_KbdKey::F11;
	case GDK_KEY_F12: return E_KbdKey::F12;
	case GDK_KEY_F13: return E_KbdKey::F13;
	case GDK_KEY_F14: return E_KbdKey::F14;
	case GDK_KEY_F15: return E_KbdKey::F15;
	case GDK_KEY_F16: return E_KbdKey::F16;
	case GDK_KEY_F17: return E_KbdKey::F17;
	case GDK_KEY_F18: return E_KbdKey::F18;
	case GDK_KEY_F19: return E_KbdKey::F19;
	case GDK_KEY_F20: return E_KbdKey::F20;
	case GDK_KEY_F21: return E_KbdKey::F21;
	case GDK_KEY_F22: return E_KbdKey::F22;
	case GDK_KEY_F23: return E_KbdKey::F23;
	case GDK_KEY_F24: return E_KbdKey::F24;
	case GDK_KEY_F25: return E_KbdKey::F25;

	case GDK_KEY_KP_0: return E_KbdKey::KP_0;
	case GDK_KEY_KP_1: return E_KbdKey::KP_1;
	case GDK_KEY_KP_2: return E_KbdKey::KP_2;
	case GDK_KEY_KP_3: return E_KbdKey::KP_3;
	case GDK_KEY_KP_4: return E_KbdKey::KP_4;
	case GDK_KEY_KP_5: return E_KbdKey::KP_5;
	case GDK_KEY_KP_6: return E_KbdKey::KP_6;
	case GDK_KEY_KP_7: return E_KbdKey::KP_7;
	case GDK_KEY_KP_8: return E_KbdKey::KP_8;
	case GDK_KEY_KP_9: return E_KbdKey::KP_9;

	case GDK_KEY_KP_Decimal: return E_KbdKey::KP_Decimal;
	case GDK_KEY_KP_Divide: return E_KbdKey::KP_Divide;
	case GDK_KEY_KP_Multiply: return E_KbdKey::KP_Mult;
	case GDK_KEY_KP_Subtract: return E_KbdKey::KP_Minus;
	case GDK_KEY_KP_Add: return E_KbdKey::KP_Plus;
	case GDK_KEY_KP_Enter: return E_KbdKey::KP_Enter;
	case GDK_KEY_KP_Equal: return E_KbdKey::KP_Equal;

	case GDK_KEY_Shift_L: return E_KbdKey::LeftShift;
	case GDK_KEY_Control_L: return E_KbdKey::LeftControl;
	case GDK_KEY_Alt_L: return E_KbdKey::LeftAlt;
	case GDK_KEY_Super_L: return E_KbdKey::LeftSuper;
	case GDK_KEY_Shift_R: return E_KbdKey::RightShift;
	case GDK_KEY_Control_R: return E_KbdKey::RightControl;
	case GDK_KEY_Alt_R: return E_KbdKey::RightAlt;
	case GDK_KEY_Super_R: return E_KbdKey::RightSuper;
	case GDK_KEY_Menu: return E_KbdKey::Menu;

	default: return E_KbdKey::Invalid;
	}
}
