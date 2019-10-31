#include "stdafx.h"
#include "SceneViewport.h"

#include <Engine/SceneRendering/SceneRenderer.h>

#include <EtEditor/Rendering/GtkRenderArea.h>
#include <EtEditor/UI/GtkUtil.h>
#include <EtEditor/SceneEditor.h>

#include <gtkmm/builder.h>


//===================
// Scene Viewport
//===================


//---------------------------
// SceneViewport::c-tor
//
SceneViewport::SceneViewport()
	: I_EditorTool()
{
	m_RefBuilder = Gtk::Builder::create_from_resource("/com/leah-lindner/editor/ui/sceneViewport.ui");
}

//---------------------------
// SceneViewport::d-tor
//
SceneViewport::~SceneViewport()
{
}

//--------------------
// SceneViewport::Init
//
// Tool initialization implementation
//
void SceneViewport::Init(EditorBase* const editor, Gtk::Frame* const parent)
{
	m_Editor = static_cast<SceneEditor*>(editor);

	// Find the GL Area widget that is responsible for rendering the scene
	SingleContextGlArea* glArea = nullptr;
	m_RefBuilder->get_widget_derived("glSceneViewport", glArea);
	ET_ASSERT(glArea != nullptr, "No 'glSceneViewport' object in sceneViewport.ui!");
	glArea->set_auto_render(true);

	// create a viewport from the area
	m_RenderArea = new GtkRenderArea(glArea);
	m_Viewport = std::make_unique<Viewport>(m_RenderArea);

	// hook up events

	// mouse click
	glArea->add_events(Gdk::BUTTON_PRESS_MASK);
	auto mousePressedCallback = [this, glArea](GdkEventButton* evnt) -> bool
	{
		E_MouseButton const code = GtkUtil::GetButtonFromGtk(evnt->button);
		if (code == E_MouseButton::Right)
		{
			// on right click we trigger picking 
			ivec2 pos = etm::vecCast<int32>(dvec2(evnt->x, evnt->y));
			pos = pos - ivec2(glArea->get_allocation().get_x(), glArea->get_allocation().get_y());

			m_Editor->GetSceneSelection().Pick(pos, m_Viewport.get(), evnt->state & GdkModifierType::GDK_CONTROL_MASK);
		}
		else
		{
			// other clicks (left) we navigate
			m_Editor->SetNavigatingViewport(this);
			m_Camera.SetEnabled(true);
			InputManager::GetInstance()->OnMousePressed(code);
		}
		return true;
	};
	glArea->signal_button_press_event().connect(mousePressedCallback, false);

	// mouse release
	glArea->add_events(Gdk::BUTTON_RELEASE_MASK);
	auto mouseReleasedCallback = [this](GdkEventButton* evnt) -> bool
	{
		InputManager::GetInstance()->OnMouseReleased(GtkUtil::GetButtonFromGtk(evnt->button));
		m_Camera.SetEnabled(false);
		m_Editor->SetNavigatingViewport(nullptr);
		return true;
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

	parent->add(*glArea);
	glArea->show(); // ensure context creation

	// create a scene renderer for the viewport
	m_SceneRenderer = new SceneRenderer();
	m_Viewport->SetRenderer(m_SceneRenderer);

	m_Editor->RegisterListener(this);
}

//--------------------
// SceneViewport::Deinit
//
void SceneViewport::Deinit(Gtk::Frame* const parent)
{
	m_Viewport->MakeCurrent();

	m_Editor->UnregisterListener(this);
	SafeDelete(m_SceneRenderer);
	m_Viewport->SetRenderer(nullptr);

	Gtk::Widget* child = parent->get_child();
	parent->remove();
	SafeDelete(child);
	m_Viewport.reset(nullptr);

	SafeDelete(m_RenderArea);
}

//---------------------------------
// SceneViewport::OnShown
//
// Show the splash screen as soon as possible
//
void SceneViewport::OnShown()
{
	m_Viewport->MakeCurrent();
	m_Viewport->SynchDimensions();
	m_SceneRenderer->InitWithSplashScreen();
	m_Viewport->Redraw();
}

//------------------------------------
// SceneViewport::OnSceneSet
//
// Once the scene is set we can start setting up rendering
//
void SceneViewport::OnSceneSet()
{
	m_Viewport->MakeCurrent();

	// Set our cameras initial position to the scenes active camera once the scene is loaded
	m_SceneInitCallback = m_Editor->GetSceneSelection().GetScene()->GetEventDispatcher().Register(E_SceneEvent::Initialized, T_SceneEventCallback(
		[this](SceneEventData const* const eventData)
		{
			m_Camera.ImitateComponent(m_Editor->GetSceneSelection().GetScene()->GetActiveCamera());
			m_Camera.PopulateCamera(m_SceneRenderer->GetCamera(), m_Viewport.get());

			m_Editor->GetSceneSelection().GetScene()->GetEventDispatcher().Unregister(m_SceneInitCallback);
		}));

	m_SceneRenderer->InitRenderingSystems();
}

//------------------------------------
// SceneViewport::OnEditorTick
//
void SceneViewport::OnEditorTick()
{
	m_Camera.Update(m_SceneRenderer->GetCamera());
	m_Camera.PopulateCamera(m_SceneRenderer->GetCamera(), m_Viewport.get());
}

//------------------------------------
// SceneViewport::OnKeyEvent
//
bool SceneViewport::OnKeyEvent(bool const pressed, GdkEventKey* const evnt)
{
	if (pressed)
	{
		InputManager::GetInstance()->OnKeyPressed(GtkUtil::GetKeyFromGtk(evnt->keyval));
	}
	else
	{
		InputManager::GetInstance()->OnKeyReleased(GtkUtil::GetKeyFromGtk(evnt->keyval));
	}

	return true;
}
