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

	m_Viewport.reset(nullptr);
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

	m_DebugFont = core::ResourceManager::Instance()->GetAssetData<render::SpriteFont>(core::HashString("Fonts/IBMPlexMono.ttf"));

	// Find the GL Area widget that is responsible for rendering the scene
	SingleContextGlArea* glArea = nullptr;
	m_RefBuilder->get_widget_derived("glSceneViewport", glArea);
	ET_ASSERT(glArea != nullptr, "No 'glSceneViewport' object in sceneViewport.ui!");
	glArea->set_auto_render(true);

	// create a viewport from the area
	m_RenderArea = new GtkRenderArea(glArea);
	m_Viewport = std::make_unique<render::Viewport>(m_RenderArea);

	// hook up events

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

			m_Editor->GetSceneSelection().Pick(pos, m_Viewport.get(), evnt->state & GdkModifierType::GDK_CONTROL_MASK);
		}
		else
		{
			// other clicks (left) we navigate
			m_Editor->SetNavigatingViewport(this);
			fw::UnifiedScene::Instance().GetEcs().GetComponent<EditorCameraComponent>(m_Camera).isEnabled = true;
			core::InputManager::GetInstance()->OnMousePressed(code);
		}
		return true;
	};
	glArea->signal_button_press_event().connect(mousePressedCallback, false);

	// mouse release
	glArea->add_events(Gdk::BUTTON_RELEASE_MASK);
	auto mouseReleasedCallback = [this](GdkEventButton* evnt) -> bool
	{
		core::InputManager::GetInstance()->OnMouseReleased(GtkUtil::GetButtonFromGtk(evnt->button));
		fw::UnifiedScene::Instance().GetEcs().GetComponent<EditorCameraComponent>(m_Camera).isEnabled = false;
		m_Editor->SetNavigatingViewport(nullptr);
		return true;
	};
	glArea->signal_button_release_event().connect(mouseReleasedCallback, false);

	// mouse moved
	glArea->add_events(Gdk::POINTER_MOTION_MASK);
	auto mouseMotionCallback = [glArea](GdkEventMotion* evnt) -> bool
	{
		// get offset of widget to window position
		ivec2 pos = math::vecCast<int32>(dvec2(evnt->x, evnt->y));
		pos = pos - ivec2(glArea->get_allocation().get_x(), glArea->get_allocation().get_y());

		core::InputManager::GetInstance()->OnMouseMoved(pos);
		return false;
	};
	glArea->signal_motion_notify_event().connect(mouseMotionCallback, false);

	// mouse scrolled
	glArea->add_events(Gdk::SMOOTH_SCROLL_MASK);
	glArea->add_events(Gdk::SCROLL_MASK);
	auto scrollCallback = [](GdkEventScroll* evnt) -> bool
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

		core::InputManager::GetInstance()->SetMouseWheelDelta(math::vecCast<int32>(delta));
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
	m_OutlineRenderer.Init(&(m_SceneRenderer->GetEventDispatcher()));

	m_IsInitialized = true;
}

//------------------------------------
// SceneViewport::OnEditorTick
//
void SceneViewport::OnEditorTick()
{
	fw::EcsController& ecs = fw::UnifiedScene::Instance().GetEcs();

	ecs.GetComponent<fw::CameraComponent>(m_Camera).PopulateCamera(m_SceneRenderer->GetCamera(), 
		*m_Viewport, 
		ecs.GetComponent<fw::TransformComponent>(m_Camera));

	if (m_DrawDebugInfo)
	{
		render::TextRenderer& textRenderer = m_SceneRenderer->GetTextRenderer();

		textRenderer.SetFont(m_DebugFont.get());
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
	if (pressed)
	{
		core::InputManager::GetInstance()->OnKeyPressed(GtkUtil::GetKeyFromGtk(evnt->keyval));
	}
	else
	{
		core::InputManager::GetInstance()->OnKeyReleased(GtkUtil::GetKeyFromGtk(evnt->keyval));
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

	EditorCameraComponent edCamComp;
	edCamComp.renderCamera = &m_SceneRenderer->GetCamera();
	m_Camera = ecs.DuplicateEntity(camEnt, edCamComp);

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
