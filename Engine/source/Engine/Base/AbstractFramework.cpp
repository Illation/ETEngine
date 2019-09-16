#include "stdafx.h"
#include "AbstractFramework.h"

#include <glad/glad.h>

#include <EtCore/FileSystem/Entry.h>
#include <EtCore/FileSystem/FileUtil.h>
#include <EtCore/FileSystem/Json/JsonParser.h>
#include <EtCore/FileSystem/Json/JsonDom.h>
#include <EtCore/Helper/Commands.h>
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

#ifdef EDITOR
#	include <Engine/Editor/Editor.h>
#endif


AbstractFramework::~AbstractFramework()
{
#ifdef EDITOR
	Editor::GetInstance()->DestroyInstance();
#endif

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
	Settings::DestroyInstance();

	ResourceManager::DestroyInstance();

	TickManager::DestroyInstance();

	Logger::Release();
}

void AbstractFramework::Run()
{
	Logger::Initialize();//Init logger first because all output depends on it from the start
#ifndef ET_SHIPPING
	DebugCopyResourceFiles();
#endif

	LoadConfig();
	PerformanceInfo::GetInstance(); // Initialize performance measurment #todo: disable for shipped project?

	m_Viewport = new Viewport(&m_RenderArea);
	m_Viewport->SetRenderer(SceneRenderer::GetInstance());
	m_RenderArea.Initialize(); // also initializes the viewport and its renderer
	m_Viewport->Redraw();

	ResourceManager::SetInstance(new PackageResourceManager());

	SceneRenderer::GetInstance()->InitWithSplashScreen();
	m_RenderArea.Update();

	InitializeGame();

	GameLoop();
}

void AbstractFramework::LoadConfig()
{
	Settings* pSet = Settings::GetInstance();//Initialize Game Settings

	File* jsonFile = new File("./config.json", nullptr);
	if (!jsonFile->Open(FILE_ACCESS_MODE::Read))
		return;

	JSON::Parser parser = JSON::Parser(FileUtil::AsText(jsonFile->Read()));
	delete jsonFile;
	jsonFile = nullptr;

	JSON::Object* root = parser.GetRoot();
	if (!root)
	{
		LOG("unable to read config json", Warning);
		return;
	}

	JSON::Object* graphics = (*root)["graphics"]->obj();
	if (graphics)
	{
		JSON::ApplyNumValue(graphics, pSet->Graphics.NumCascades, "CSM Cascade Count");
		JSON::ApplyNumValue(graphics, pSet->Graphics.NumPCFSamples, "PCF Sample Count");
		JSON::ApplyNumValue(graphics, pSet->Graphics.CSMDrawDistance, "CSM Draw Distance");
		JSON::ApplyNumValue(graphics, pSet->Graphics.PbrBrdfLutSize, "PBR BRDF LUT Resolution");
		JSON::ApplyNumValue(graphics, pSet->Graphics.TextureScaleFactor, "Texture Scale Factor");
		JSON::ApplyNumValue(graphics, pSet->Graphics.NumBlurPasses, "Bloom Blur Passes");
		JSON::ApplyBoolValue(graphics, pSet->Graphics.UseFXAA, "Use FXAA");
	}
	JSON::Object* window = (*root)["window"]->obj();
	if (window)
	{
		JSON::ApplyBoolValue(window, pSet->Window.Fullscreen, "Fullscreen");
		JSON::ApplyStrValue(window, pSet->Window.Title, "Title");
		std::vector<ivec2> resolutions;
		JSON::Value* jval = (*window)["Resolutions"];
		if (jval)
		{
			for (JSON::Value* res : jval->arr()->value)
			{
				ivec2 vec;
				if (JSON::ArrayVector(res, vec))resolutions.push_back(vec);
			}
		}
		uint32 resIdx;
		if (JSON::ApplyNumValue(window, resIdx, pSet->Window.Fullscreen ? "Fullscreen Resolution" : "Windowed Resolution"))
		{
			if (resIdx < (uint32)resolutions.size()) 
				pSet->Window.Resize(resolutions[resIdx].x, resolutions[resIdx].y, false);
		}
	}
	AudioManager::GetInstance()->Initialize();//Initialize Audio
	PhysicsManager::GetInstance()->Initialize();//Initialize Physics
	SceneManager::GetInstance();//Initialize SceneManager
	AddScenes();
	JSON::String* initialScene = (*root)["start scene"]->str();
	if (initialScene)
	{
		SceneManager::GetInstance()->SetActiveGameScene(initialScene->value);
	}
	//Screenshot manager
	JSON::String* screenshotDir = (*root)["screenshot dir"]->str();
	if (screenshotDir)
	{
		ScreenshotCapture::GetInstance()->Initialize(screenshotDir->value);
	}
	else
	{
		ScreenshotCapture::GetInstance()->Initialize("./");
	}
}

void AbstractFramework::InitializeGame()
{
	InputManager::GetInstance();	// init input manager
	GlfwEventManager::GetInstance()->Init(m_RenderArea.GetWindow());

	RenderPipeline::GetInstance()->Initialize();

#ifdef EDITOR
	Editor::GetInstance()->Initialize();
#endif

	RegisterAsTriggerer();
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