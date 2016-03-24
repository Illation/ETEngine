#include "stdafx.hpp"
#include "MainFramework.hpp"

#include "../SceneGraph/SceneManager.hpp"

#include "Scenes/TestScene.hpp"
#include "Scenes/SkyboxTestScene.hpp"

MainFramework::MainFramework() :
	AbstractFramework()
{
}
MainFramework::~MainFramework()
{
}

void MainFramework::Initialize()
{
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