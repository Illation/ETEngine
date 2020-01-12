#include "stdafx.h"
#include "AbstractFramework.h"

#include <EtCore/Helper/PerformanceInfo.h>
#include <EtCore/UpdateCycle/TickManager.h>
#include <EtRendering/GraphicsContext/Viewport.h>
#include <EtRendering/SceneRendering/ShadedSceneRenderer.h>
#include <EtRendering/SceneRendering/SplashScreenRenderer.h>
#include <EtRendering/SceneRendering/ShadowRenderer.h>

#include <EtFramework/SceneGraph/UnifiedScene.h>
#include <EtFramework/Physics/PhysicsManager.h>
#include <EtFramework/Audio/AudioManager.h>
#include <EtFramework/Components/CameraComponent.h>

#include <EtRuntime/Core/GlfwEventManager.h>
#include <EtRuntime/Core/PackageResourceManager.h>


AbstractFramework::~AbstractFramework()
{
	GlfwEventManager::DestroyInstance();
	m_RenderArea.Uninitialize();
	SafeDelete(m_Viewport);
	SafeDelete(m_SceneRenderer);

	fw::UnifiedScene::Instance().UnloadScene();

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

	// init unified scene, systems etc
	fw::UnifiedScene::Instance().GetEventDispatcher().Register(E_SceneEvent::RegisterSystems,
		T_SceneEventCallback([this](T_SceneEventFlags const flags, SceneEventData const* const eventData)
		{
			UNUSED(flags);
			UNUSED(eventData);

			OnSystemInit();
		}));

	// ensure we show the splash screen every time the scene switches
	// Callback ID not required as we destroy the scene manager here before destroying this class
	fw::UnifiedScene::Instance().GetEventDispatcher().Register(E_SceneEvent::SceneSwitch | E_SceneEvent::Activated,
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

	fw::UnifiedScene::Instance().Init();

	// init rendering target
	m_Viewport = new Viewport(&m_RenderArea);
	m_SplashScreenRenderer = new render::SplashScreenRenderer();
	m_Viewport->SetRenderer(m_SplashScreenRenderer);
	m_RenderArea.Initialize(); // also initializes the viewport and its renderer

	// screenshots
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

	// resources
	ResourceManager::SetInstance(new PackageResourceManager());

	cfg->InitRenderConfig();

	m_SplashScreenRenderer->Init();
	m_RenderArea.Update();

	AudioManager::GetInstance()->Initialize();
	PhysicsManager::GetInstance()->Initialize();

	PerformanceInfo::GetInstance(); // Initialize performance measurment #todo: disable for shipped project?

	// init input manager
	InputManager::GetInstance();	
	GlfwEventManager::GetInstance()->Init(&m_RenderArea);

	// scene rendering
	m_SceneRenderer = new render::ShadedSceneRenderer(&(fw::UnifiedScene::Instance().GetRenderScene()));
	m_SceneRenderer->InitRenderingSystems();

	// cause the loop to contine
	RegisterAsTriggerer();

	// load scene
	OnInit();
	std::string const& initScene = Config::GetInstance()->GetStartScene();
	if (!initScene.empty())
	{
		fw::UnifiedScene::Instance().LoadScene(GetHash(initScene + ".json"));
	}

	// update
	MainLoop();
}

void AbstractFramework::MainLoop()
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

		fw::EcsController& ecs = fw::UnifiedScene::Instance().GetEcs();
		fw::T_EntityId cam = fw::UnifiedScene::Instance().GetActiveCamera();

		ecs.GetComponent<fw::CameraComponent>(cam).PopulateCamera(m_SceneRenderer->GetCamera(),
			*m_Viewport,
			ecs.GetComponent<fw::TransformComponent>(cam));

		m_RenderArea.Update();
	}
}
