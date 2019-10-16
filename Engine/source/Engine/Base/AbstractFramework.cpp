#include "stdafx.h"
#include "AbstractFramework.h"

#include <EtCore/FileSystem/Entry.h>
#include <EtCore/FileSystem/FileUtil.h>
#include <EtCore/FileSystem/Json/JsonParser.h>
#include <EtCore/FileSystem/Json/JsonDom.h>
#include <EtCore/Helper/PerformanceInfo.h>
#include <EtCore/UpdateCycle/TickManager.h>
#include <EtCore/Content/PackageResourceManager.h>

#include <Engine/SceneGraph/SceneManager.h>
#include <Engine/Physics/PhysicsManager.h>
#include <Engine/Audio/AudioManager.h>
#include <Engine/Helper/GlfwEventManager.h>
#include <Engine/GraphicsContext/Viewport.h>
#include <Engine/SceneRendering/SceneRenderer.h>
#include <Engine/SceneRendering/ShadowRenderer.h>


AbstractFramework::~AbstractFramework()
{
	GlfwEventManager::DestroyInstance();
	m_RenderArea.Uninitialize();
	SafeDelete(m_Viewport);
	SafeDelete(m_SceneRenderer);

	SceneManager::DestroyInstance();

	PhysicsManager::DestroyInstance();
	AudioManager::DestroyInstance();

	InputManager::DestroyInstance();
	ContextManager::DestroyInstance();

	PerformanceInfo::DestroyInstance();
	
	ResourceManager::DestroyInstance();

	TickManager::DestroyInstance();

	Logger::Release();
}

void AbstractFramework::Run()
{
	Logger::Initialize();//Init logger first because all output depends on it from the start

	Config::GetInstance()->Initialize();

	m_Viewport = new Viewport(&m_RenderArea);
	m_SceneRenderer = new SceneRenderer();
	m_Viewport->SetRenderer(m_SceneRenderer);
	m_RenderArea.Initialize(); // also initializes the viewport and its renderer
	m_Viewport->Redraw();

	ResourceManager::SetInstance(new PackageResourceManager());

	m_SceneRenderer->InitWithSplashScreen();
	m_RenderArea.Update();

	AudioManager::GetInstance()->Initialize();
	PhysicsManager::GetInstance()->Initialize();

	// set up scene manager
	SceneManager::GetInstance();
	AddScenes();
	std::string const& initScene = Config::GetInstance()->GetStartScene();
	if (!initScene.empty())
	{
		SceneManager::GetInstance()->SetActiveGameScene(initScene);
	}

	PerformanceInfo::GetInstance(); // Initialize performance measurment #todo: disable for shipped project?

	InputManager::GetInstance();	// init input manager
	GlfwEventManager::GetInstance()->Init(m_RenderArea.GetWindow());

	m_SceneRenderer->InitRenderingSystems();

	RegisterAsTriggerer();

	GameLoop();
}

void AbstractFramework::GameLoop()
{
	while (true)
	{
		if (!(InputManager::GetInstance()->IsRunning()))
		{
			return;
		}
		TriggerTick(); // this will probably tick the scene manager, editor, framework etc

		//****
		//DRAW

		m_RenderArea.Update();
	}
}