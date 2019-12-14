#include "stdafx.h"
#include "MainFramework.h"

#include <EtFramework/SceneGraph/SceneManager.h>

#include <Runtime/Scenes/EditorScene.h>
#include <Runtime/Scenes/PlanetTestScene.h>
#include <Runtime/Scenes/PhysicsTestScene.h>


//================
// Main Framework
//================


//--------------------------
// MainFramework::AddScenes
//
// Init function
//
void MainFramework::AddScenes()
{
	SceneManager* const sceneMan = SceneManager::GetInstance();

	sceneMan->AddScene(new EditorScene());
	sceneMan->AddScene(new PlanetTestScene());
	sceneMan->AddScene(new PhysicsTestScene());
	
	sceneMan->GetEventDispatcher().Register(E_SceneEvent::Activated,
		T_SceneEventCallback([this](T_SceneEventFlags const flags, SceneEventData const* const evnt)
		{
			UNUSED(flags);
			UNUSED(evnt);

			CameraComponent* const cam = CAMERA;
			ET_ASSERT(evnt->scene->GetActiveCamera() == cam);

			m_CameraController.SetCameraComponent(cam);
			m_CameraController.Reset();
		}));
}

//--------------------------
// MainFramework::OnTick
//
// Demo specific updates indepenant of scene
//
void MainFramework::OnTick()
{
	SceneManager* const sceneMan = SceneManager::GetInstance();
	InputManager* const input = InputManager::GetInstance();

	// Scene switching
	//-----------------
	if(input->GetKeyState(E_KbdKey::F3) == E_KeyState::Pressed)
	{
		sceneMan->PreviousScene();
	}

	if(input->GetKeyState(E_KbdKey::F4) == E_KeyState::Pressed)
	{
		sceneMan->NextScene();
	}

	// Screenshots
	//-------------
	if (input->GetKeyState(E_KbdKey::F12) == E_KeyState::Pressed)
	{
		m_ScreenshotCapture.Take(Viewport::GetCurrentViewport());
	}

	// view
	//------
	m_CameraController.Update();

	bool const up = (input->GetKeyState(E_KbdKey::Up) == E_KeyState::Down);
	if (up || (input->GetKeyState(E_KbdKey::Down) == E_KeyState::Down))
	{
		render::Scene& renderScene = sceneMan->GetRenderScene();
		PostProcessingSettings ppSettings = renderScene.GetPostProcessingSettings();

		float const newExp = ppSettings.exposure * 4.f;
		ppSettings.exposure += (newExp - ppSettings.exposure) * TIME->DeltaTime() * (up ? 1.f : -1.f);

		LOG(FS("Exposure: %f", ppSettings.exposure));

		renderScene.SetPostProcessingSettings(ppSettings);
	}
}