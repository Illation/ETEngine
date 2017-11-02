#include "stdafx.hpp"
#include "AbstractFramework.hpp"

#include <FreeImage.h>

#include "../SceneGraph/SceneManager.hpp"
#include "../GraphicsHelper/LightVolume.hpp"
#include "../GraphicsHelper/ShadowRenderer.hpp"
#include "../GraphicsHelper/TextRenderer.hpp"
#include "../Helper/PerformanceInfo.hpp"
#include "../GraphicsHelper/PrimitiveRenderer.hpp"
#include "../GraphicsHelper/RenderPipeline.hpp"


void FreeImageErrorHandler(FREE_IMAGE_FORMAT fif, const char *message) {
	printf("\n*** ");
	if (fif != FIF_UNKNOWN) {
		printf("%s Format\n", FreeImage_GetFormatFromFIF(fif));
	}
	printf(message);
	printf(" ***\n");
}
AbstractFramework::AbstractFramework()
{
}
AbstractFramework::~AbstractFramework()
{
	Logger::Release();

	ContentManager::Release();

	FreeImage_DeInitialise();

	SDL_GL_DeleteContext(m_GlContext);
	SDL_Quit();

	Settings::GetInstance()->DestroyInstance();
	SceneManager::GetInstance()->DestroyInstance();
	InputManager::GetInstance()->DestroyInstance();
	Context::GetInstance()->DestroyInstance();
	
	RenderPipeline::GetInstance()->DestroyInstance();
}

void AbstractFramework::Run()
{
	InitializeSDL();
	InitializeWindow();
	InitializeUtilities();
	BindOpenGL();
	InitializeDebug();
	InitializeGame();

	GameLoop();
}



void AbstractFramework::InitializeSDL()
{
	// Request an OpenGL 4.5 context (should be core)
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 5);
	// Also request a depth buffer
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
	// Request a debug context.
#if defined(DEBUG) | defined(_DEBUG)
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG);
#endif

	// Initialize SDL 
	if (SDL_Init(SDL_INIT_VIDEO) < 0)
		quit_SDL_error("Couldn't initialize SDL");
	atexit(SDL_Quit);
	SDL_GL_LoadLibrary(NULL);
}

void AbstractFramework::InitializeWindow()
{
	Settings* pSet = Settings::GetInstance();//Initialize Game Settings
	PerformanceInfo::GetInstance();//Initialize performance measurment #todo: disable for shipped project?
	SceneManager::GetInstance();//Initialize SceneManager
	InputManager::GetInstance()->Init();//init input manager

	//Create Window
	if (pSet->Window.Fullscreen)
	{
		pSet->Window.pWindow = SDL_CreateWindow(pSet->Window.Title.c_str(), SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, pSet->Window.Width, pSet->Window.Height, SDL_WINDOW_OPENGL | SDL_WINDOW_FULLSCREEN_DESKTOP);
	}
	else
	{
		pSet->Window.pWindow = SDL_CreateWindow(pSet->Window.Title.c_str(), SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, pSet->Window.Width, pSet->Window.Height, SDL_WINDOW_OPENGL);
	}
	if (pSet->Window.pWindow == NULL)quit_SDL_error("Couldn't set video mode");

	//OpenGl context creation
	m_GlContext = SDL_GL_CreateContext(pSet->Window.pWindow);
	if (m_GlContext == NULL)quit_SDL_error("Failed to create OpenGL context");

	// Use v-sync
	SDL_GL_SetSwapInterval(1);
}

void AbstractFramework::InitializeUtilities()
{
	FreeImage_Initialise();
	#ifdef _DEBUG
		FreeImage_SetOutputMessage(FreeImageErrorHandler);
	#endif
}

void AbstractFramework::BindOpenGL()
{
	// Check OpenGL properties and create open gl function pointers
	std::cout << "OpenGL loaded" << std::endl << std::endl;
	gladLoadGLLoader(SDL_GL_GetProcAddress);
	std::cout << "Vendor: \t" << glGetString(GL_VENDOR) << std::endl;
	std::cout << "Renderer:\t" << glGetString(GL_RENDERER) << std::endl;
	std::cout << "Version:\t" << glGetString(GL_VERSION) << std::endl << std::endl;
}

void AbstractFramework::InitializeDebug()
{
	Logger::Initialize();
	// Enable the debug callback -- is now done by logger
}

void AbstractFramework::InitializeGame()
{
	//Initialize Managers
	ContentManager::Initialize();

	RenderPipeline::GetInstance()->Initialize();

	SceneManager::GetInstance()->Initialize();

	//Initialize Game
	Initialize();
}

void AbstractFramework::GameLoop()
{
	while (true)
	{
		InputManager::GetInstance()->UpdateEvents();
		if (InputManager::GetInstance()->IsExitRequested())return;

		std::vector<AbstractScene*> activeScenes;
		if (SceneManager::GetInstance()->GetActiveScene())
		{
			activeScenes.push_back(SceneManager::GetInstance()->GetActiveScene());
		}
		// #note: currently only one scene but could be expanded for nested scenes

		Update();
		//******
		//UPDATE
		SceneManager::GetInstance()->Update();

		//****
		//DRAW
		RenderPipeline::GetInstance()->Draw(activeScenes);
	}
}

void AbstractFramework::ClearTarget()
{
	// Clear the screen to white
	STATE->SetClearColor(vec4(1));
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}