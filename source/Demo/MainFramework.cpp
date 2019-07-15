#include "stdafx.h"

#include "MainFramework.h"

#include <Engine/SceneGraph/SceneManager.h>

#include <Demo/Scenes/TestScene.h>
#include <Demo/Scenes/SkyboxTestScene.h>
#include <Demo/Scenes/ShadingTestScene.h>
#include <Demo/Scenes/PlanetTestScene.h>

#include <Demo/Scenes/PhysicsTestScene.h>

#include <btBulletCollisionCommon.h>


MainFramework::MainFramework() :
	AbstractFramework()
{
}
MainFramework::~MainFramework()
{
}

void MainFramework::AddScenes()
{
	SceneManager::GetInstance()->AddGameScene(new PlanetTestScene());
	SceneManager::GetInstance()->AddGameScene(new PhysicsTestScene());
	SceneManager::GetInstance()->AddGameScene(new ShadingTestScene());
	SceneManager::GetInstance()->AddGameScene(new SkyboxTestScene());
	SceneManager::GetInstance()->AddGameScene(new TestScene());
}

void MainFramework::OnTick()
{	
	//This is where scenes can be switched conditionally
	if(INPUT->GetKeyState(static_cast<uint32>(SDLK_F3)) == E_KeyState::Pressed)
	{
		SceneManager::GetInstance()->PreviousScene();
	}

	if(INPUT->GetKeyState(static_cast<uint32>(SDLK_F4)) == E_KeyState::Pressed)
	{
		SceneManager::GetInstance()->NextScene();
	}
}