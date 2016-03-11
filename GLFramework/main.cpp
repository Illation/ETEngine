#include "staticDependancies/glad/glad.h"

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

#include "Base\Context.hpp"
#include "Base\Settings.hpp"
#include "Game\Objects\Cube.hpp"
#include "Graphics\FrameBuffer.hpp"
#include "Graphics\Camera.hpp"


//Static stuff
static void sdl_die(const char * message) 
{
	fprintf(stderr, "%s: %s\n", message, SDL_GetError());
	exit(2);
}
static void APIENTRY openglCallbackFunction(GLenum source,GLenum type,GLuint id,GLenum severity,GLsizei length,const GLchar* message,const void* userParam) 
{
	(void)source; (void)type; (void)id;
	(void)severity; (void)length; (void)userParam;
	std::cout << std::endl;
	fprintf(stderr, "%s\n", message);
	if (severity == GL_DEBUG_SEVERITY_HIGH) 
	{
		fprintf(stderr, "Aborting...\n");
		abort();
	}
	std::cout << std::endl;
}

int wmain(int argc, char *argv[])
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

	Settings* pSet = Settings::GetInstance();//Initialize Game Settings
	Context* pCon = Context::GetInstance();//Initialize Game Context

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
	SDL_GLContext context = SDL_GL_CreateContext(pSet->Window.pWindow);
	if (context == NULL)sdl_die("Failed to create OpenGL context");

	// Use v-sync
	SDL_GL_SetSwapInterval(1);

	//Init DevIL
	ilInit();
	iluInit();

	// Check OpenGL properties and create open gl function pointers
	std::cout << "OpenGL loaded" << std::endl << std::endl;
	gladLoadGLLoader(SDL_GL_GetProcAddress);
	std::cout << "Vendor: \t" << glGetString(GL_VENDOR) << std::endl;
	std::cout << "Renderer:\t" << glGetString(GL_RENDERER) << std::endl;
	std::cout << "Version:\t" << glGetString(GL_VERSION) << std::endl << std::endl;

	// Enable the debug callback
#ifdef _DEBUG
	glEnable(GL_DEBUG_OUTPUT);
	glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
	glDebugMessageCallback(openglCallbackFunction, nullptr);
	glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, NULL, true);
#endif

	//--------------------------
	//Cube Test
	//--------------------------
	Camera* pMainCam = new Camera();
	pMainCam->SetActive();
	pMainCam->SetFieldOfView(45);
	pMainCam->SetNearClippingPlane(1);
	pMainCam->SetFarClippingPlane(10);
	pMainCam->UsePerspectiveProjection();

	Cube* pTestCube = new Cube(); 
	pTestCube->Initialize();
	
	FrameBuffer* pPostProc = new FrameBuffer();
	pPostProc->Initialize();

	//Render Buffer for depth and stencil
	GLuint rboDepthStencil;
	glGenRenderbuffers(1, &rboDepthStencil);
	glBindRenderbuffer(GL_RENDERBUFFER, rboDepthStencil);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, pSet->Window.Width, pSet->Window.Height);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rboDepthStencil);

	pCon->pTime->Start();

	SDL_Event windowEvent; //Main Loop
	while (true)
	{
		if (SDL_PollEvent(&windowEvent))
		{
			if (windowEvent.type == SDL_QUIT) break;
			if (windowEvent.type == SDL_KEYUP &&
				windowEvent.key.keysym.sym == SDLK_ESCAPE) break;
		}

		//Bind Framebuffer and draw 3D scene
		pPostProc->Enable();

		// Clear the screen to white
		glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		pMainCam->Update();
		
		pTestCube->Update();
		pTestCube->Draw();

		// Bind default framebuffer and draw contents of our framebuffer
		pPostProc->Enable(false);
		pPostProc->Draw();

		//Swap front and back buffer
		SDL_GL_SwapWindow(pSet->Window.pWindow);

		//Update Time
		pCon->pTime->Update();
		//std::cout << pCon->pTime->FPS() << std::endl;
	}

	//Delete the stuff, no memory leakerino
	glDeleteRenderbuffers(1, &rboDepthStencil);
	delete pTestCube;
	delete pPostProc;
	delete pMainCam;

	SDL_GL_DeleteContext(context);
	SDL_Quit();

	pCon->DestroyInstance();
	pSet->DestroyInstance();

	return 0;
}