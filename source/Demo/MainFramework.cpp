#include "stdafx.h"

#include "MainFramework.h"

#include <EtCore/Content/ResourceManager.h>
#include <EtCore/Content/AssetStub.h>

#include <Engine/Graphics/Shader.h>
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
	if (m_PostDefShader == nullptr)
	{
		m_PostDefShader = ResourceManager::GetInstance()->GetAssetData<ShaderData>("PostDeferredComposite.glsl"_hash);
	}

	if (m_LoremTxt == nullptr)
	{
		m_LoremTxt = ResourceManager::GetInstance()->GetAssetData<StubData>("lorem_ipsum.txt"_hash);
	}
	AssetPtr<StubData> temp = m_LoremTxt;
	AssetPtr<StubData> temp2(temp);
	
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