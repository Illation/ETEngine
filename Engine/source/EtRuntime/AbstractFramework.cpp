#include "stdafx.h"
#include "AbstractFramework.h"

#include <EtCore/Helper/PerformanceInfo.h>
#include <EtCore/UpdateCycle/TickManager.h>
#include <EtRendering/GraphicsContext/Viewport.h>
#include <EtRendering/SceneRendering/ShadedSceneRenderer.h>
#include <EtRendering/SceneRendering/SplashScreenRenderer.h>
#include <EtRendering/SceneRendering/ShadowRenderer.h>

#include <EtFramework/SceneGraph/SceneManager.h>
#include <EtFramework/SceneGraph/UnifiedScene.h>
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
	//Logger::StartFileLogging("debug_log.log");

	Config* const cfg = Config::GetInstance();
	cfg->Initialize();

	framework::UnifiedScene::Instance().Init();
	SceneManager::GetInstance();
	m_Viewport = new Viewport(&m_RenderArea);
	m_SplashScreenRenderer = new render::SplashScreenRenderer();
	m_Viewport->SetRenderer(m_SplashScreenRenderer);
	m_RenderArea.Initialize(); // also initializes the viewport and its renderer

	std::string const& screenshotDir = cfg->GetScreenshotDir();
	if (!screenshotDir.empty())
	{
		m_ScreenshotCapture.Initialize(cfg->GetUserDirPath() + screenshotDir);
	}
	else
	{
		m_ScreenshotCapture.Initialize(cfg->GetUserDirPath() + std::string("./"));
	}

	m_Viewport->SynchDimensions();
	m_Viewport->Redraw();

	ResourceManager::SetInstance(new PackageResourceManager());

	cfg->InitRenderConfig();

	m_SplashScreenRenderer->Init();
	m_RenderArea.Update();

	AudioManager::GetInstance()->Initialize();
	PhysicsManager::GetInstance()->Initialize();

	// set up scene manager
	AddScenes();
	std::string const& initScene = Config::GetInstance()->GetStartScene();
	if (!initScene.empty())
	{
		SceneManager::GetInstance()->SetActiveGameScene(initScene);
		//framework::UnifiedScene::Instance().LoadScene(GetHash(initScene + ".json"));
	}

	// ensure we show the splash screen every time the scene switches
	// Callback ID not required as we destroy the scene manager here before destroying this class
	SceneManager::GetInstance()->GetEventDispatcher().Register(E_SceneEvent::SceneSwitch | E_SceneEvent::Activated, 
		T_SceneEventCallback([this](T_SceneEventFlags const flags, SceneEventData const* const evnt)
		{
			UNUSED(evnt);

			switch (static_cast<E_SceneEvent>(flags))
			{
			case E_SceneEvent::SceneSwitch:
				m_Viewport->SetRenderer(m_SplashScreenRenderer);

				m_Viewport->SetTickDisabled(true);
				m_RenderArea.Update(); // update manually incase we don't run the game loop before the new scene is activated 
				m_Viewport->SetTickDisabled(false);
				break;

			case E_SceneEvent::Activated:
				m_Viewport->SetRenderer(m_SceneRenderer); // update will happen anyway during the loop
				break;

			default:
				ET_ASSERT(true, "Unexpected scene event type!");
				break;
			}
		}));

	PerformanceInfo::GetInstance(); // Initialize performance measurment #todo: disable for shipped project?

	InputManager::GetInstance();	// init input manager
	GlfwEventManager::GetInstance()->Init(&m_RenderArea);

	m_SceneRenderer = new render::ShadedSceneRenderer(&(SceneManager::GetInstance()->GetRenderScene()));
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
			SceneManager::GetInstance()->GetActiveScene()->GetActiveCamera()->PopulateCamera(m_SceneRenderer->GetCamera(), *m_Viewport);
		}

		m_RenderArea.Update();
	}
}
