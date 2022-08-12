#include "stdafx.h"
#include "AbstractFramework.h"

#include <EtBuild/EngineVersion.h>

#include <EtCore/UpdateCycle/PerformanceInfo.h>
#include <EtCore/UpdateCycle/TickManager.h>
#include <EtCore/Reflection/TypeInfoRegistry.h>
#include <EtCore/Trace/NetworkTraceHandler.h>
#if ET_CT_IS_ENABLED(ET_CT_DBG_UTIL)
#	include <EtCore/Hashing/HashStringRegistry.h>
#endif

#include <EtRHI/GraphicsContext/Viewport.h>
#include <EtRHI/GraphicsContext/ContextHolder.h>

#include <EtRendering/SceneRendering/ShadedSceneRenderer.h>
#include <EtRendering/SceneRendering/ShadowRenderer.h>

#include <EtGUI/Context/RmlGlobal.h>

#include <EtFramework/SceneGraph/UnifiedScene.h>
#include <EtFramework/Physics/PhysicsManager.h>
#include <EtFramework/Audio/AudioManager.h>
#include <EtFramework/Components/CameraComponent.h>
#include <EtFramework/Config/BootConfig.h>

#include <EtApplication/Core/GlfwEventManager.h>
#include <EtApplication/Core/PackageResourceManager.h>

#include <EtRuntime/SplashScreenRenderer.h>


namespace et {
namespace rt {


//====================
// Abstract Framework
//====================


//----------------------------
// AbstractFramework::c-tor
//
AbstractFramework::AbstractFramework() 
	: core::I_Tickable(static_cast<uint32>(fw::E_TickOrder::TICK_Framework))
#if ET_CT_IS_ENABLED(ET_CT_IMGUI)
	, m_ImguiBackend(static_cast<uint32>(fw::E_TickOrder::TICK_ImguiBackend))
#endif
{ }

//----------------------------
// AbstractFramework::d-tor
//
// Deinit all systems
//
AbstractFramework::~AbstractFramework()
{
	fw::UnifiedScene::Instance().UnloadScene();

#if ET_CT_IS_ENABLED(ET_CT_IMGUI)
	m_ImguiBackend.Deinit();
#endif

	m_GuiRenderer.Deinit();

	fw::UnifiedScene::Instance().Deinit();

	gui::RmlGlobal::Destroy();
	m_SceneRenderer = nullptr;
	m_SplashScreenRenderer = nullptr;
	m_Viewport->SetRenderer(nullptr);

	app::GlfwEventManager::DestroyInstance();
	m_RenderWindow->GetArea().Uninitialize();
	m_Viewport = nullptr;

	fw::PhysicsManager::DestroyInstance();
	fw::AudioManager::DestroyInstance();

	core::InputManager::DestroyInstance();
	core::ContextManager::DestroyInstance();

#if ET_CT_IS_ENABLED(ET_CT_DBG_UTIL)
	core::PerformanceInfo::DestroyInstance();
#endif
	
	core::ResourceManager::DestroyInstance();

	core::TickManager::DestroyInstance();

	core::TraceService::Destroy();
}

//------------------------
// AbstractFramework::Run
//
// Bootstrap the engine and start the main loop
//
void AbstractFramework::Run()
{
	// init trace handling
	core::TraceService::Initialize(); // Init trace first because all output depends on it from the start
#ifdef ET_SHIPPING
	core::TraceService::Instance()->AddHandler<core::FileTraceHandler>("debug_log.log");
#else
	core::TraceService::Instance()->SetupDefaultHandlers("ET Engine Instance", true);
#endif

	// now we can print
	ET_LOG_I(ET_CTX_RUNTIME, "E.T.Engine");
	ET_LOG_I(ET_CTX_RUNTIME, "//////////");
	ET_LOG_I(ET_CTX_RUNTIME, "");
	ET_LOG_I(ET_CTX_RUNTIME, " - version: %s", et::build::Version::s_Name.c_str());
	ET_LOG_I(ET_CTX_RUNTIME, "");

	core::TypeInfoRegistry::Instance().Initialize(); // this needs to be initialized ASAP because serialization depends on it

	// now we can deserialize the configuration
	fw::Config* const cfg = fw::Config::GetInstance();
	cfg->Initialize();

	core::NetworkTraceHandler* const netTraceHandler = core::TraceService::Instance()->GetHandler<core::NetworkTraceHandler>();
	if (netTraceHandler != nullptr)
	{
		netTraceHandler->UpdateClientName(cfg->GetWindow().m_Title);
	}

	// init unified scene, systems etc
	fw::UnifiedScene& unifiedScene = fw::UnifiedScene::Instance();
	unifiedScene.GetEventDispatcher().Register(fw::E_SceneEvent::RegisterSystems,
		fw::T_SceneEventCallback([this](fw::T_SceneEventFlags const flags, fw::SceneEventData const* const eventData)
		{
			ET_UNUSED(flags);
			ET_UNUSED(eventData);

			OnSystemInit();
		}));

	// ensure we show the splash screen every time the scene switches
	// Callback ID not required as we destroy the scene manager here before destroying this class
	unifiedScene.GetEventDispatcher().Register(fw::E_SceneEvent::SceneSwitch | fw::E_SceneEvent::Activated | fw::E_SceneEvent::ActiveCameraChanged,
		fw::T_SceneEventCallback([this](fw::T_SceneEventFlags const flags, fw::SceneEventData const* const evnt)
		{
			switch (static_cast<fw::E_SceneEvent>(flags))
			{
			case fw::E_SceneEvent::SceneSwitch:
				m_SplashScreenRenderer->SetGuiDocument(m_SplashScreenGui);
				m_Viewport->SetRenderer(m_SplashScreenRenderer);

				m_Viewport->SetTickDisabled(true);
				m_RenderWindow->GetArea().Update(); // update manually incase we don't run the game loop before the new scene is activated 
				m_Viewport->SetTickDisabled(false);
				break;

			case fw::E_SceneEvent::Activated:
				m_Viewport->SetRenderer(m_SceneRenderer); // update will happen anyway during the loop
				m_SplashScreenRenderer->SetGuiDocument(core::HashString());
				break;

			case fw::E_SceneEvent::ActiveCameraChanged:
			{
				fw::CameraComponent& camera = evnt->scene->GetEcs().GetComponent<fw::CameraComponent>(evnt->scene->GetActiveCamera());
				camera.SetViewport(ToPtr(m_Viewport.Get()));
				m_SceneRenderer->SetCamera(camera.GetId());
				break;
			}

			default:
				ET_ASSERT(true, "Unexpected scene event type!");
				break;
			}
		}));

	unifiedScene.Init();

	// init rendering target
	m_RenderWindow = Create<app::GlfwRenderWindow>(cfg->GetWindow(), false);
	m_Viewport = Create<rhi::Viewport>(ToPtr(&m_RenderWindow->GetArea()));
	m_Viewport->SetTickDisabled(true);
	m_SplashScreenRenderer = Create<rt::SplashScreenRenderer>();
	m_Viewport->SetRenderer(m_SplashScreenRenderer);
	rhi::ContextHolder::Instance().CreateMainRenderContext(ToPtr(m_RenderWindow.Get())); // also initializes the viewport and its renderer

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
	app::PackageResourceManager* const pkgResMan = new app::PackageResourceManager();
	core::ResourceManager::SetInstance(pkgResMan);

	fw::BootConfig bootCfg;
	fw::BootConfig::LoadFromPackage(bootCfg, pkgResMan->GetRootPackage());
	m_Scenes = std::move(bootCfg.allScenes);

	cfg->InitRenderConfig();

	m_SplashScreenRenderer->Init();
	m_SplashScreenGui = bootCfg.splashGui;
	m_SplashScreenRenderer->SetGuiDocument(bootCfg.splashGui);
	m_RenderWindow->GetArea().Update();

	fw::AudioManager::GetInstance()->Initialize();
	fw::AudioManager::GetInstance()->SetDistanceModel(AL_INVERSE_DISTANCE);

	fw::PhysicsManager::GetInstance()->Initialize();

#if ET_CT_IS_ENABLED(ET_CT_DBG_UTIL)
	core::PerformanceInfo::GetInstance(); 
	core::HashStringRegistry::Instance().InitDebugCommands();
#endif

	// init input 
	core::InputManager* const inputMan = core::InputManager::GetInstance();
	app::GlfwEventManager& glfwMan = *app::GlfwEventManager::GetInstance();
	m_Viewport->SetInputProvider(ToPtr(&glfwMan.GetInputProvider()));
	glfwMan.Init(ToPtr(&m_RenderWindow->GetArea()));
	glfwMan.GetInputProvider().RegisterListener(ToPtr(inputMan));

	// scene rendering
	m_SceneRenderer = Create<render::ShadedSceneRenderer>(&(unifiedScene.GetRenderScene()));
	m_SceneRenderer->InitRenderingSystems();

	// ui rendering
	m_GuiRenderer.Init(ToPtr(&(m_SceneRenderer->GetEventDispatcher())));
	gui::RmlGlobal::GetInstance()->SetCursorShapeManager(ToPtr(&glfwMan));
	gui::RmlGlobal::GetInstance()->SetClipboardController(ToPtr(&glfwMan));

	// cause the loop to continue
	RegisterAsTriggerer();

	// load scene
	OnInit();
	unifiedScene.LoadScene(bootCfg.startScene);

#if ET_CT_IS_ENABLED(ET_CT_IMGUI)
	m_ImguiBackend.Init(ToPtr(&glfwMan), ToPtr(&glfwMan), ToPtr(m_Viewport.Get()));
#endif

	m_Viewport->SetTickDisabled(false);

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

		m_RenderWindow->GetArea().Update();
	}
}


} // namespace rt
} // namespace et
