#include "stdafx.h"
#include "MainFramework.h"

#include <EtFramework/SceneGraph/SceneManager.h>

#include <Runtime/Scenes/EditorScene.h>
#include <Runtime/Scenes/TestScene.h>
#include <Runtime/Scenes/SkyboxTestScene.h>
#include <Runtime/Scenes/ShadingTestScene.h>
#include <Runtime/Scenes/PlanetTestScene.h>
#include <Runtime/Scenes/PhysicsTestScene.h>

//#include <btBulletCollisionCommon.h>


MainFramework::MainFramework() :
	AbstractFramework()
{
}
MainFramework::~MainFramework()
{
}

void MainFramework::AddScenes()
{
	SceneManager::GetInstance()->AddGameScene(new EditorScene());
	SceneManager::GetInstance()->AddGameScene(new PlanetTestScene());
	SceneManager::GetInstance()->AddGameScene(new PhysicsTestScene());
	//SceneManager::GetInstance()->AddGameScene(new ShadingTestScene());
	//SceneManager::GetInstance()->AddGameScene(new SkyboxTestScene());
	//SceneManager::GetInstance()->AddGameScene(new TestScene());
}

void MainFramework::OnTick()
{	
	// Scene switching
	if(INPUT->GetKeyState(E_KbdKey::F3) == E_KeyState::Pressed)
	{
		SceneManager::GetInstance()->PreviousScene();
	}

	if(INPUT->GetKeyState(E_KbdKey::F4) == E_KeyState::Pressed)
	{
		SceneManager::GetInstance()->NextScene();
	}

	// Screenshots
	if (INPUT->GetKeyState(E_KbdKey::F12) == E_KeyState::Pressed)
	{
		m_ScreenshotCapture.Take(Viewport::GetCurrentViewport());
	}

	// Exposure control
	bool const up = (INPUT->GetKeyState(E_KbdKey::Up) == E_KeyState::Down);
	if (up || (INPUT->GetKeyState(E_KbdKey::Down) == E_KeyState::Down))
	{
		render::Scene& renderScene = SceneManager::GetInstance()->GetRenderScene();
		PostProcessingSettings ppSettings = renderScene.GetPostProcessingSettings();

		float const newExp = ppSettings.exposure * 4.f;
		ppSettings.exposure += (newExp - ppSettings.exposure) * TIME->DeltaTime() * (up ? 1.f : -1.f);

		LOG(FS("Exposure: %f", ppSettings.exposure));

		renderScene.SetPostProcessingSettings(ppSettings);
	}
}