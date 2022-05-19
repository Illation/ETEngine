#include "stdafx.h"
#include "AbstractFramework.h"

#include <EtBuild/EngineVersion.h>

#include <EtCore/UpdateCycle/PerformanceInfo.h>
#include <EtCore/UpdateCycle/TickManager.h>
#include <EtCore/Reflection/TypeInfoRegistry.h>

#include <EtRendering/GraphicsContext/Viewport.h>
#include <EtRendering/GraphicsContext/ContextHolder.h>
#include <EtRendering/SceneRendering/ShadedSceneRenderer.h>
#include <EtRendering/SceneRendering/ShadowRenderer.h>

#include <EtGUI/Context/RmlGlobal.h>

#include <EtFramework/SceneGraph/UnifiedScene.h>
#include <EtFramework/Physics/PhysicsManager.h>
#include <EtFramework/Audio/AudioManager.h>
#include <EtFramework/Components/CameraComponent.h>
#include <EtFramework/Config/BootConfig.h>

#include <EtRuntime/Rendering/SplashScreenRenderer.h>
#include <EtRuntime/Core/GlfwEventManager.h>
#include <EtRuntime/Core/PackageResourceManager.h>


namespace et {
namespace rt {


//----------------------------
// AbstractFramework::d-tor
//
// Deinit all systems
//
AbstractFramework::~AbstractFramework()
{
	fw::UnifiedScene::Instance().UnloadScene();

	m_GuiRenderer.Deinit();
	gui::RmlGlobal::Destroy();
	m_SceneRenderer = nullptr;
	m_SplashScreenRenderer = nullptr;
	m_Viewport->SetRenderer(nullptr);

	GlfwEventManager::DestroyInstance();
	m_RenderWindow.GetArea().Uninitialize();
	m_Viewport = nullptr;

	fw::PhysicsManager::DestroyInstance();
	fw::AudioManager::DestroyInstance();

	core::InputManager::DestroyInstance();
	core::ContextManager::DestroyInstance();

	core::PerformanceInfo::DestroyInstance();
	
	core::ResourceManager::DestroyInstance();

	core::TickManager::DestroyInstance();

	core::Logger::Release();
}

//------------------------
// AbstractFramework::Run
//
// Bootstrap the engine and start the main loop
//
void AbstractFramework::Run()
{
	core::Logger::Initialize();//Init logger first because all output depends on it from the start
	//Logger::StartFileLogging("debug_log.log");

	LOG(FS("E.T.Engine"));
	LOG(FS("//////////"));
	LOG("");
	LOG(FS(" - version: %s", et::build::Version::s_Name.c_str()));
	LOG("");

	core::TypeInfoRegistry::Instance().Initialize(); // this needs to be initialized ASAP because serialization depends on it

	fw::Config* const cfg = fw::Config::GetInstance();
	cfg->Initialize();

	// init unified scene, systems etc
	fw::UnifiedScene& unifiedScene = fw::UnifiedScene::Instance();
	unifiedScene.GetEventDispatcher().Register(fw::E_SceneEvent::RegisterSystems,
		fw::T_SceneEventCallback([this](fw::T_SceneEventFlags const flags, fw::SceneEventData const* const eventData)
		{
			UNUSED(flags);
			UNUSED(eventData);

			OnSystemInit();
		}));

	// ensure we show the splash screen every time the scene switches
	// Callback ID not required as we destroy the scene manager here before destroying this class
	unifiedScene.GetEventDispatcher().Register(fw::E_SceneEvent::SceneSwitch | fw::E_SceneEvent::Activated,
		fw::T_SceneEventCallback([this](fw::T_SceneEventFlags const flags, fw::SceneEventData const* const evnt)
		{
			UNUSED(evnt);

			switch (static_cast<fw::E_SceneEvent>(flags))
			{
			case fw::E_SceneEvent::SceneSwitch:
				m_Viewport->SetRenderer(m_SplashScreenRenderer.Get());

				m_Viewport->SetTickDisabled(true);
				m_RenderWindow.GetArea().Update(); // update manually incase we don't run the game loop before the new scene is activated 
				m_Viewport->SetTickDisabled(false);
				break;

			case fw::E_SceneEvent::Activated:
				m_Viewport->SetRenderer(m_SceneRenderer.Get()); // update will happen anyway during the loop
				break;

			default:
				ET_ASSERT(true, "Unexpected scene event type!");
				break;
			}
		}));

	unifiedScene.Init();

	// init rendering target
	m_Viewport = Create<render::Viewport>(&m_RenderWindow.GetArea());
	m_SplashScreenRenderer = Create<rt::SplashScreenRenderer>();
	m_Viewport->SetRenderer(m_SplashScreenRenderer.Get());
	render::ContextHolder::Instance().CreateMainRenderContext(&m_RenderWindow); // also initializes the viewport and its renderer

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
	PackageResourceManager* const pkgResMan = new PackageResourceManager();
	core::ResourceManager::SetInstance(pkgResMan);

	fw::BootConfig bootCfg;
	fw::BootConfig::LoadFromPackage(bootCfg, pkgResMan->GetRootPackage());
	m_Scenes = std::move(bootCfg.allScenes);

	cfg->InitRenderConfig();

	m_SplashScreenRenderer->Init();
	m_RenderWindow.GetArea().Update();

	fw::AudioManager::GetInstance()->Initialize();
	fw::PhysicsManager::GetInstance()->Initialize();

	core::PerformanceInfo::GetInstance(); // Initialize performance measurment #todo: disable for shipped project?

	// init input manager
	core::InputManager::GetInstance();	
	GlfwEventManager::GetInstance()->Init(&m_RenderWindow.GetArea());

	// scene rendering
	m_SceneRenderer = Create<render::ShadedSceneRenderer>(&(unifiedScene.GetRenderScene()));
	m_SceneRenderer->InitRenderingSystems();

	// ui rendering
	m_GuiRenderer.Init(ToPtr(&(m_SceneRenderer->GetEventDispatcher())));
	m_GuiContext = unifiedScene.GetGuiExtension()->GetContextContainer().CreateContext(m_Viewport);
	unifiedScene.SetScreenGuiContext(m_GuiContext);

	// cause the loop to continue
	RegisterAsTriggerer();

	// load scene
	OnInit();
	unifiedScene.LoadScene(bootCfg.startScene);

	// update
	MainLoop();
}

//-----------------------------
// AbstractFramework::MainLoop
//
// Main update entry point
//
void AbstractFramework::MainLoop()
{
	while (true)
	{
		if (!(core::InputManager::GetInstance()->IsRunning()))
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

		m_RenderWindow.GetArea().Update();
	}
}


} // namespace rt
} // namespace et
