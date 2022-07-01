#include "stdafx.h"
#include "SceneViewport.h"

#include <gtkmm/builder.h>

#include <EtCore/Content/ResourceManager.h>

#include <EtRendering/SceneRendering/ShadedSceneRenderer.h>

#include <EtFramework/SceneGraph/UnifiedScene.h>
#include <EtFramework/Components/AudioListenerComponent.h>

#include <EtEditor/Rendering/EditorCamera.h>
#include <EtEditor/Rendering/GtkRenderArea.h>
#include <EtEditor/Util/GtkUtil.h>
#include <EtEditor/SceneEditor/SceneEditor.h>


namespace et {
namespace edit {


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
	if (m_IsInitialized)
	{
		OnDeinit();
	}

	m_Viewport = nullptr;
	SafeDelete(m_RenderArea);
}

//--------------------
// SceneViewport::Init
//
// Tool initialization implementation
//
void SceneViewport::Init(EditorBase* const editor, Gtk::Frame* const parent)
{
	m_Editor = static_cast<SceneEditor*>(editor);

	m_DebugFont = core::ResourceManager::Instance()->GetAssetData<gui::SdfFont>(core::HashString("Fonts/IBMPlexMono.ttf"));

	// Find the GL Area widget that is responsible for rendering the scene
	SingleContextGlArea* glArea = nullptr;
	m_RefBuilder->get_widget_derived("glSceneViewport", glArea);
	ET_ASSERT(glArea != nullptr, "No 'glSceneViewport' object in sceneViewport.ui!");
	glArea->set_auto_render(true);

	// create a viewport from the area
	m_RenderArea = new GtkRenderArea(glArea);
	m_Viewport = Create<render::Viewport>(m_RenderArea);
	m_Viewport->SetInputProvider(ToPtr(&m_InputProvider));

	// hook up events

	m_InputProvider.RegisterListener(ToPtr(core::InputManager::GetInstance()));

	// mouse click
	glArea->add_events(Gdk::BUTTON_PRESS_MASK);
	auto mousePressedCallback = [this, glArea](GdkEventButton* evnt) -> bool
	{
		E_MouseButton const code = GtkUtil::GetButtonFromGtk(evnt->button);
		if (code == E_MouseButton::Right)
		{
			// on right click we trigger picking 
			ivec2 pos = math::vecCast<int32>(dvec2(evnt->x, evnt->y));
			pos = pos - ivec2(glArea->get_allocation().get_x(), glArea->get_allocation().get_y());

			m_Editor->GetSceneSelection().Pick(pos, m_Viewport.Get(), evnt->state & GdkModifierType::GDK_CONTROL_MASK);
		}
		else
		{
			// other clicks (left) we navigate
			m_Editor->SetNavigatingViewport(this);
			fw::UnifiedScene::Instance().GetEcs().GetComponent<EditorCameraComponent>(m_Camera).isEnabled = true;

			core::T_KeyModifierFlags const mods = GtkUtil::GetModifiersFromGtk(evnt->state);
			m_InputProvider.IterateListeners(core::RawInputProvider::T_EventFn([code, mods](core::I_RawInputListener& listener)
				{
					return listener.ProcessMousePressed(code, mods);
				}));
		}

		return true;
	};
	glArea->signal_button_press_event().connect(mousePressedCallback, false);

	// mouse release
	glArea->add_events(Gdk::BUTTON_RELEASE_MASK);
	auto mouseReleasedCallback = [this](GdkEventButton* evnt) -> bool
	{
		E_MouseButton const code = GtkUtil::GetButtonFromGtk(evnt->button);
		core::T_KeyModifierFlags const mods = GtkUtil::GetModifiersFromGtk(evnt->state);
		m_InputProvider.IterateListeners(core::RawInputProvider::T_EventFn([code, mods](core::I_RawInputListener& listener)
			{
				return listener.ProcessMouseReleased(code, mods);
			}));

		fw::UnifiedScene::Instance().GetEcs().GetComponent<EditorCameraComponent>(m_Camera).isEnabled = false;
		m_Editor->SetNavigatingViewport(nullptr);
		return true;
	};
	glArea->signal_button_release_event().connect(mouseReleasedCallback, false);

	// mouse moved
	glArea->add_events(Gdk::POINTER_MOTION_MASK);
	auto mouseMotionCallback = [glArea, this](GdkEventMotion* evnt) -> bool
	{
		// get offset of widget to window position
		ivec2 pos = math::vecCast<int32>(dvec2(evnt->x, evnt->y));
		pos = pos - ivec2(glArea->get_allocation().get_x(), glArea->get_allocation().get_y());

		core::T_KeyModifierFlags const mods = GtkUtil::GetModifiersFromGtk(evnt->state);
		m_InputProvider.IterateListeners(core::RawInputProvider::T_EventFn([pos, mods](core::I_RawInputListener& listener)
			{
				return listener.ProcessMouseMove(pos, mods);
			}));

		return false;
	};
	glArea->signal_motion_notify_event().connect(mouseMotionCallback, false);

	// mouse scrolled
	glArea->add_events(Gdk::SMOOTH_SCROLL_MASK);
	glArea->add_events(Gdk::SCROLL_MASK);
	auto scrollCallback = [this](GdkEventScroll* evnt) -> bool
	{
		dvec2 delta(evnt->delta_x, evnt->delta_y);
		if (math::isZero(delta))
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

		core::T_KeyModifierFlags const mods = GtkUtil::GetModifiersFromGtk(evnt->state);
		m_InputProvider.IterateListeners(core::RawInputProvider::T_EventFn([delta, mods](core::I_RawInputListener& listener)
			{
				return listener.ProcessMouseWheelDelta(math::vecCast<int32>(delta), mods);
			}));

		return false;
	};
	glArea->signal_scroll_event().connect(scrollCallback, false);

	parent->add(*glArea);
	glArea->show(); // ensure context creation

	// create a scene renderer for the viewport
	m_SceneRenderer = new render::ShadedSceneRenderer(&(fw::UnifiedScene::Instance().GetRenderScene()));
	m_Viewport->SetRenderer(m_SceneRenderer);

	m_Editor->RegisterListener(this);

	// in case the editor is already initialized at this point (we switch a tool to being a scene viewport at runtime) - do further init steps
	if (m_Editor->IsShown())
	{
		OnShown();
	}

	if (fw::UnifiedScene::Instance().GetSceneId() != 0u)
	{
		OnSceneSet();
	}
}

//----------------------------
// SceneViewport::OnDeinit
//
void SceneViewport::OnDeinit()
{
	m_Viewport->MakeCurrent();
	m_Viewport->SetActive(false);

	m_Editor->UnregisterListener(this);

	m_OutlineRenderer.Deinit();
	m_SceneGuiRenderer.Deinit();
	SafeDelete(m_SceneRenderer);
	m_Viewport->SetRenderer(nullptr);

	if (m_Camera != fw::INVALID_ENTITY_ID)
	{
		fw::UnifiedScene::Instance().GetEcs().RemoveEntity(m_Camera);
	}

	m_IsInitialized = false;
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
	//m_SceneRenderer->InitWithSplashScreen();
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

	if (fw::UnifiedScene::Instance().GetSceneId() != 0u)
	{
		InitCamera();
	}
	else
	{
		m_SceneInitCallback = fw::UnifiedScene::Instance().GetEventDispatcher().Register(fw::E_SceneEvent::Activated,
			fw::T_SceneEventCallback( [this](fw::T_SceneEventFlags const flags, fw::SceneEventData const* const eventData)
			{
				UNUSED(flags);
				UNUSED(eventData);

				InitCamera();

				fw::UnifiedScene::Instance().GetEventDispatcher().Unregister(m_SceneInitCallback);
			}));
	}

	m_SceneRenderer->InitRenderingSystems();
	m_SceneGuiRenderer.Init(ToPtr(&(m_SceneRenderer->GetEventDispatcher())));
	m_OutlineRenderer.Init(ToPtr(&(m_SceneRenderer->GetEventDispatcher())));

	m_IsInitialized = true;
}

//------------------------------------
// SceneViewport::OnEditorTick
//
void SceneViewport::OnEditorTick()
{
	if (m_DrawDebugInfo)
	{
		gui::TextRenderer& textRenderer = m_SceneGuiRenderer.GetTextRenderer();

		textRenderer.SetFont(m_DebugFont);
		textRenderer.SetColor(vec4(1, 0.3f, 0.3f, 1));
		std::string outString = FS("FPS: %i", core::PerformanceInfo::GetInstance()->GetRegularFPS());
		textRenderer.DrawText(outString, vec2(10, 32), 22);
	}
}

//------------------------------------
// SceneViewport::OnKeyEvent
//
bool SceneViewport::OnKeyEvent(bool const pressed, GdkEventKey* const evnt)
{
	E_KbdKey const key = GtkUtil::GetKeyFromGtk(evnt->keyval);
	core::T_KeyModifierFlags const mods = GtkUtil::GetModifiersFromGtk(evnt->state);
	if (pressed)
	{
		core::E_Character const character = static_cast<core::E_Character>(gdk_keyval_to_unicode(evnt->keyval));
		if (!m_InputProvider.IterateListeners(core::RawInputProvider::T_EventFn([character](core::I_RawInputListener& listener)
			{
				return listener.ProcessTextInput(character);
			})))
		{
			m_InputProvider.IterateListeners(core::RawInputProvider::T_EventFn([key, mods](core::I_RawInputListener& listener)
				{
					return listener.ProcessKeyPressed(key, mods);
				}));
		}
	}
	else
	{
		m_InputProvider.IterateListeners(core::RawInputProvider::T_EventFn([key, mods](core::I_RawInputListener& listener)
			{
				return listener.ProcessKeyReleased(key, mods);
			}));
	}

	return true;
}

//------------------------------------
// SceneViewport::InitCamera
//
void SceneViewport::InitCamera()
{
	fw::EcsController& ecs = fw::UnifiedScene::Instance().GetEcs();
	fw::T_EntityId const camEnt = fw::UnifiedScene::Instance().GetActiveCamera();

	m_Camera = ecs.DuplicateEntity(camEnt, EditorCameraComponent());
	fw::CameraComponent& camera = ecs.GetComponent<fw::CameraComponent>(m_Camera);
	camera.SetViewport(ToPtr(m_Viewport.Get()));
	m_SceneRenderer->SetCamera(camera.GetId());

	if (ecs.HasComponent<fw::AudioListenerComponent>(camEnt))
	{
		ecs.RemoveComponents<fw::AudioListenerComponent>(camEnt);
	}

	if (ecs.HasComponent<fw::ActiveAudioListenerComponent>(camEnt))
	{
		ecs.RemoveComponents<fw::ActiveAudioListenerComponent>(camEnt);
	}
}


} // namespace edit
} // namespace et
