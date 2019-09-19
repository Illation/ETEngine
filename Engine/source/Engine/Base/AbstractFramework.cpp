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
#include <Engine/GraphicsHelper/LightVolume.h>
#include <Engine/GraphicsHelper/ShadowRenderer.h>
#include <Engine/GraphicsHelper/TextRenderer.h>
#include <Engine/GraphicsHelper/PrimitiveRenderer.h>
#include <Engine/GraphicsHelper/RenderPipeline.h>
#include <Engine/Physics/PhysicsManager.h>
#include <Engine/Audio/AudioManager.h>
#include <Engine/Helper/GlfwEventManager.h>
#include <Engine/Helper/ScreenshotCapture.h>
#include <Engine/GraphicsHelper/Viewport.h>
#include <Engine/GraphicsHelper/SceneRenderer.h>


AbstractFramework::~AbstractFramework()
{
	GlfwEventManager::DestroyInstance();
	m_RenderArea.Uninitialize();
	SafeDelete(m_Viewport);
	SceneRenderer::DestroyInstance();

	SceneManager::DestroyInstance();

	PhysicsManager::DestroyInstance();
	AudioManager::DestroyInstance();

	InputManager::DestroyInstance();
	ContextManager::DestroyInstance();
	
	RenderPipeline::DestroyInstance();

	ResourceManager::DestroyInstance();

	TickManager::DestroyInstance();

	Logger::Release();
}

void AbstractFramework::Run()
{
	Logger::Initialize();//Init logger first because all output depends on it from the start

	Config::GetInstance()->Initialize();

	m_Viewport = new Viewport(&m_RenderArea);
	m_Viewport->SetRenderer(SceneRenderer::GetInstance());
	m_RenderArea.Initialize(); // also initializes the viewport and its renderer
	m_Viewport->Redraw();

	ResourceManager::SetInstance(new PackageResourceManager());

	SceneRenderer::GetInstance()->InitWithSplashScreen();
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

	// set up screenshot manager
	std::string const& screenshotDir = Config::GetInstance()->GetScreenshotDir();
	if (!screenshotDir.empty())
	{
		ScreenshotCapture::GetInstance()->Initialize(Config::GetInstance()->GetUserDirPath() + screenshotDir);
	}
	else
	{
		ScreenshotCapture::GetInstance()->Initialize(Config::GetInstance()->GetUserDirPath() + std::string("./"));
	}

	PerformanceInfo::GetInstance(); // Initialize performance measurment #todo: disable for shipped project?

	InputManager::GetInstance();	// init input manager
	GlfwEventManager::GetInstance()->Init(m_RenderArea.GetWindow());

	RenderPipeline::GetInstance()->Initialize();

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