#include "stdafx.h"
#include "AbstractFramework.h"

#include <EtCore/Helper/PerformanceInfo.h>
#include <EtCore/UpdateCycle/TickManager.h>
#include <EtRendering/GraphicsContext/Viewport.h>
#include <EtRendering/SceneRendering/ShadedSceneRenderer.h>
#include <EtRendering/SceneRendering/ShadowRenderer.h>

#include <EtFramework/SceneGraph/SceneManager.h>
#include <EtFramework/Physics/PhysicsManager.h>
#include <EtFramework/Audio/AudioManager.h>

#include <EtRuntime/Core/GlfwEventManager.h>
#include <EtRuntime/Core/PackageResourceManager.h>


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

	SceneManager::GetInstance();
	m_Viewport = new Viewport(&m_RenderArea);
	m_SceneRenderer = new render::ShadedSceneRenderer(&(SceneManager::GetInstance()->GetRenderScene()));
	m_Viewport->SetRenderer(m_SceneRenderer);
	m_RenderArea.Initialize(); // also initializes the viewport and its renderer
	m_Viewport->SynchDimensions();
	m_Viewport->Redraw();

	ResourceManager::SetInstance(new PackageResourceManager());

	//m_SceneRenderer->InitWithSplashScreen();
	//m_RenderArea.Update();

	AudioManager::GetInstance()->Initialize();
	PhysicsManager::GetInstance()->Initialize();

	// set up scene manager
	AddScenes();
	std::string const& initScene = Config::GetInstance()->GetStartScene();
	if (!initScene.empty())
	{
		SceneManager::GetInstance()->SetActiveGameScene(initScene);
	}

	PerformanceInfo::GetInstance(); // Initialize performance measurment #todo: disable for shipped project?

	InputManager::GetInstance();	// init input manager
	GlfwEventManager::GetInstance()->Init(&m_RenderArea);

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

		if (SceneManager::GetInstance()->GetActiveScene()->IsInitialized())
		{
			SceneManager::GetInstance()->GetActiveScene()->GetActiveCamera()->PopulateCamera(m_SceneRenderer->GetCamera());
		}

		m_RenderArea.Update();
	}
}
