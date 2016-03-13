#include "AbstractFramework.hpp"

#include <SDL.h>
#include <SDL_opengl.h>

#include <IL/il.h>
#include <IL/ilu.h>
#include <IL/ilut.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>
#include <fstream>
#include <string>
#include <chrono>

#include "Context.hpp"
#include "Settings.hpp"
#include "InputManager.hpp"
#include "../Game\Objects\Cube.hpp"
#include "../Graphics\FrameBuffer.hpp"
#include "../SceneGraph/SceneManager.hpp"

AbstractFramework::AbstractFramework()
{
}
AbstractFramework::~AbstractFramework()
{
	Settings* pSet = Settings::GetInstance();
	Context* pCon = Context::GetInstance();
	SceneManager* pScMan = SceneManager::GetInstance();
	InputManager* pInMan = InputManager::GetInstance();

	SDL_GL_DeleteContext(m_GlContext);
	SDL_Quit();

	pSet->DestroyInstance();
	pScMan->DestroyInstance();
	pInMan->DestroyInstance();
	pCon->DestroyInstance();
}

void AbstractFramework::Run()
{
	InitializeSDL();
	InitializeWindow();
	InitializeDevIL();
	BindOpenGL();
	InitializeDebug();
	InitializeGame();

	GameLoop();
}



void AbstractFramework::InitializeSDL()
{
	// Initialize SDL 
	if (SDL_Init(SDL_INIT_VIDEO) < 0)
		sdl_die("Couldn't initialize SDL");
	atexit(SDL_Quit);
	SDL_GL_LoadLibrary(NULL);

	// Request an OpenGL 4.5 context (should be core)
	SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 5);
	// Also request a depth buffer
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
	// Request a debug context.
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG);
}

void AbstractFramework::InitializeWindow()
{
	Settings* pSet = Settings::GetInstance();//Initialize Game Settings
	SceneManager* pScMan = SceneManager::GetInstance();//Initialize SceneManager
	InputManager* pInMan = InputManager::GetInstance();//init input manager
	pInMan->Init();

	//Create Window
	if (pSet->Window.Fullscreen)
	{
		pSet->Window.pWindow = SDL_CreateWindow(pSet->Window.Title.c_str(), SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, pSet->Window.Width, pSet->Window.Height, SDL_WINDOW_OPENGL | SDL_WINDOW_FULLSCREEN_DESKTOP);
	}
	else
	{
		pSet->Window.pWindow = SDL_CreateWindow(pSet->Window.Title.c_str(), SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, pSet->Window.Width, pSet->Window.Height, SDL_WINDOW_OPENGL);
	}
	if (pSet->Window.pWindow == NULL)sdl_die("Couldn't set video mode");

	//OpenGl context creation
	m_GlContext = SDL_GL_CreateContext(pSet->Window.pWindow);
	if (m_GlContext == NULL)sdl_die("Failed to create OpenGL context");

	// Use v-sync
	SDL_GL_SetSwapInterval(1);
}

void AbstractFramework::InitializeDevIL()
{
	//Init DevIL
	ilInit();
	iluInit();
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
	// Enable the debug callback
#ifdef _DEBUG
	glEnable(GL_DEBUG_OUTPUT);
	glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
	glDebugMessageCallback(openglCallbackFunction, nullptr);
	glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, NULL, true);
#endif
}

void AbstractFramework::InitializeGame()
{
	//Initialize Managers
	SceneManager::GetInstance()->Initialize();

	//Initialize Game
	Initialize();
}

void AbstractFramework::GameLoop()
{
	Settings* pSet = Settings::GetInstance();
	while (true)
	{
		InputManager::GetInstance()->UpdateEvents();
		if (InputManager::GetInstance()->IsExitRequested())return;

		Update();
		//******
		//UPDATE
		SceneManager::GetInstance()->Update();

		//****
		//DRAW
		SceneManager::GetInstance()->Draw();

		//Swap front and back buffer
		SDL_GL_SwapWindow(pSet->Window.pWindow);
	}
}

void AbstractFramework::ClearTarget()
{
	// Clear the screen to white
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}