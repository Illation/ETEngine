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
	//SceneManager::GetInstance()->AddGameScene(new PlanetTestScene());
	SceneManager::GetInstance()->AddGameScene(new PhysicsTestScene());
	//SceneManager::GetInstance()->AddGameScene(new ShadingTestScene());
	//SceneManager::GetInstance()->AddGameScene(new SkyboxTestScene());
	//SceneManager::GetInstance()->AddGameScene(new TestScene());
}

void MainFramework::OnTick()
{	
	//This is where scenes can be switched conditionally
	if(INPUT->GetKeyState(E_KbdKey::F3) == E_KeyState::Pressed)
	{
		SceneManager::GetInstance()->PreviousScene();
	}

	if(INPUT->GetKeyState(E_KbdKey::F4) == E_KeyState::Pressed)
	{
		SceneManager::GetInstance()->NextScene();
	}

	if (INPUT->GetKeyState(E_KbdKey::F12) == E_KeyState::Pressed)
	{
		m_ScreenshotCapture.Take(Viewport::GetCurrentViewport());
	}
}