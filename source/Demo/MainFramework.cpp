#include "stdafx.hpp"
#include "MainFramework.hpp"

#include "../Engine/SceneGraph/SceneManager.hpp"

#include "Scenes/TestScene.hpp"
#include "Scenes/SkyboxTestScene.hpp"
#include "Scenes/ShadingTestScene.hpp"
#include "Scenes/PlanetTestScene.hpp"

#include <btBulletCollisionCommon.h>
#include "PhysicsTestScene.h"

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

void MainFramework::Update()
{	
	//This is where scenes can be switched conditionally
	if (INPUT->IsKeyboardKeyPressed(SDL_SCANCODE_F3))
	{
		SceneManager::GetInstance()->PreviousScene();
	}
	if (INPUT->IsKeyboardKeyPressed(SDL_SCANCODE_F4))
	{
		SceneManager::GetInstance()->NextScene();
	}
}