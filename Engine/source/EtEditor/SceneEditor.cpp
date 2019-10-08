#include "stdafx.h"
#include "SceneEditor.h"

#include "EditorScene.h"

#include <EtEditor/Rendering/GtkRenderArea.h>
#include <EtEditor/UI/GtkUtil.h>

#include <EtCore/Helper/InputManager.h>

#include <Engine/SceneGraph/SceneManager.h>
#include <Engine/GraphicsHelper/SceneRenderer.h>
#include <Engine/Physics/PhysicsManager.h>
#include <Engine/Audio/AudioManager.h>

#include <gtkmm/paned.h>
#include <gtkmm/glarea.h>
#include <gtkmm/box.h>


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
	SceneRenderer::DestroyInstance();
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
	m_RefBuilder = Gtk::Builder::create_from_resource("/com/leah-lindner/editor/ui/outliner.ui");

	// get the toplevel element
	Gtk::Paned* paned = nullptr;
	m_RefBuilder->get_widget("sceneEditor", paned);
	ET_ASSERT(paned != nullptr);

	// listen for keyboard input
	// on press
	auto keyPressedCallback = [this](GdkEventKey* evnt) -> bool
	{
		if (m_IsNavigating)
		{
			InputManager::GetInstance()->OnKeyPressed(GetKeyFromGtk(evnt->keyval));
			return true;
		}

		return false;
	};
	parent->signal_key_press_event().connect(keyPressedCallback, false);

	// on release
	auto keyReleasedCallback = [](GdkEventKey* evnt) -> bool
	{
		InputManager::GetInstance()->OnKeyReleased(GetKeyFromGtk(evnt->keyval));
		return true;
	};
	parent->signal_key_release_event().connect(keyReleasedCallback, false);

	m_SceneViewport = std::move(CreateSceneViewport());
	m_Outliner = std::move(CreateOutliner());

	parent->add(*paned);
	paned->show_all_children();

	SceneRenderer::GetInstance()->InitWithSplashScreen();
	m_SceneViewport->Redraw();

	AudioManager::GetInstance()->Initialize();
	PhysicsManager::GetInstance()->Initialize();

	SceneManager::GetInstance()->AddGameScene(new EditorScene());
	SceneManager::GetInstance()->SetActiveGameScene("EditorScene");
	m_SceneSelection.SetScene(SceneManager::GetInstance()->GetNewActiveScene());

	SceneRenderer::GetInstance()->InitRenderingSystems();
}

//---------------------------------
// SceneEditor::CreateSceneViewport
//
// Create a viewport with an openGL area in it
//
std::unique_ptr<Viewport> SceneEditor::CreateSceneViewport()
{
	// Find the GL Area widget that is responsible for rendering the scene
	Gtk::GLArea* glArea = nullptr;
	m_RefBuilder->get_widget("glSceneViewport", glArea);
	ET_ASSERT(glArea != nullptr, "No 'glSceneViewport' object in editorWindow.ui!");
	glArea->set_auto_render(true);

	// create a viewport from the area
	std::unique_ptr<Viewport> viewport = std::make_unique<Viewport>(new GtkRenderArea(glArea));

	Viewport* const viewportCptr = viewport.get();

	// hook up events
	// mouse click
	glArea->add_events(Gdk::BUTTON_PRESS_MASK);
	auto mousePressedCallback = [this, glArea, viewportCptr](GdkEventButton* evnt) -> bool
	{
		E_MouseButton const code = GetButtonFromGtk(evnt->button);
		if (code == E_MouseButton::Right)
		{
			// on right click we trigger picking 
			ivec2 pos = etm::vecCast<int32>(dvec2(evnt->x, evnt->y));
			pos = pos - ivec2(glArea->get_allocation().get_x(), glArea->get_allocation().get_y());

			m_SceneSelection.Pick(pos, viewportCptr, evnt->state & GdkModifierType::GDK_CONTROL_MASK);
		}
		else
		{
			// other clicks (left) we navigate
			m_IsNavigating = true;
			InputManager::GetInstance()->OnMousePressed(code);
		}
		return false;
	};
	glArea->signal_button_press_event().connect(mousePressedCallback, false);

	// mouse release
	glArea->add_events(Gdk::BUTTON_RELEASE_MASK);
	auto mouseReleasedCallback = [this](GdkEventButton* evnt) -> bool
	{
		InputManager::GetInstance()->OnMouseReleased(GetButtonFromGtk(evnt->button));
		m_IsNavigating = false;
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
// SceneEditor::CreateOutliner
//
std::unique_ptr<Outliner> SceneEditor::CreateOutliner()
{
	Gtk::Paned* propertySpace = nullptr;
	m_RefBuilder->get_widget("propertySpace", propertySpace);
	ET_ASSERT(propertySpace != nullptr);

	Gtk::Frame* childFrame = Gtk::make_managed<Gtk::Frame>();
	childFrame->set_shadow_type(Gtk::SHADOW_ETCHED_IN);

	propertySpace->add2(*childFrame);

	std::unique_ptr<Outliner> outliner = std::make_unique<Outliner>(&m_SceneSelection, childFrame);

	propertySpace->set_position(200);

	childFrame->show_all_children();

	return std::move(outliner);
}

//---------------------------------
// SceneEditor::GetButtonFromGtk
//
// Convert GTK mouse button codes to the input managers buttons
//
E_MouseButton SceneEditor::GetButtonFromGtk(uint32 const buttonCode)
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
// SceneEditor::GetButtonFromGtk
//
// Convert GTK key codes to the input managers keys
//
E_KbdKey SceneEditor::GetKeyFromGtk(uint32 const keyCode)
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

