#include "stdafx.hpp"
#include "MainFramework.hpp"

#include "../SceneGraph/SceneManager.hpp"

#include "Scenes/TestScene.hpp"

MainFramework::MainFramework() :
	AbstractFramework()
{
}
MainFramework::~MainFramework()
{
}

void MainFramework::Initialize()
{
	SceneManager::GetInstance()->AddGameScene(new TestScene());
}

void MainFramework::Update()
{	
	//This is where scenes can be switched conditionally
}