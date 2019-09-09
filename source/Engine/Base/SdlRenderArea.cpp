#include "stdafx.h"
#include "SdlRenderArea.h"

#include <glad/glad.h>


//---------------------------------
// SdlRenderArea::Initialize
//
// Create a Window and an openGL context to draw to the window
//
void SdlRenderArea::Initialize()
{
	// Initialize SDL 
	//----------------
	if (SDL_Init(SDL_INIT_VIDEO) < 0)
	{
		LOG(FS("Couldn't initialize SDL: %s", SDL_GetError()), LogLevel::Error);
	}

	SDL_GL_LoadLibrary(nullptr);

	// Create Window
	//---------------

	// Set the OopenGL context attributes before creating the window
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 5);
	// Also request a depth buffer
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

#if defined(GRAPHICS_API_DEBUG)
	// Request a debug context.
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG);
#endif

	Settings* const settings = Settings::GetInstance();

	uint32 WindowFlags = SDL_WINDOW_OPENGL;
	if (settings->Window.Fullscreen)
	{
		WindowFlags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
	}
	else
	{
		WindowFlags |= SDL_WINDOW_RESIZABLE;
	}

#ifdef EDITOR
	ivec2 const dim = settings->Window.EditorDimensions;
#else
	ivec2 const dim = settings->Window.Dimensions;
#endif

	m_Window = SDL_CreateWindow(settings->Window.Title.c_str(), SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, dim.x, dim.y, WindowFlags);

	if (m_Window == nullptr)
	{
		LOG(FS("Couldn't set video mode: %s", SDL_GetError()), LogLevel::Error);
	}

	// OpenGl context creation
	//-------------------------

	m_Context = SDL_GL_CreateContext(m_Window);
	if (m_Context == nullptr)
	{
		LOG(FS("Failed to create OpenGL context: %s", SDL_GetError()), LogLevel::Error);
	}

	// Use v-sync
	SDL_GL_SetSwapInterval(1);

	// Check OpenGL properties and create open gl function pointers
	//--------------------------------------------------------------
	gladLoadGLLoader(SDL_GL_GetProcAddress);

	LOG("OpenGL loaded");
	LOG("");
	LOG(FS("Vendor:   %s", glGetString(GL_VENDOR)));
	LOG(FS("Renderer: %s", glGetString(GL_RENDERER)));
	LOG(FS("Version:  %s", glGetString(GL_VERSION)));
	LOG("");

	// potentially hook up opengl to the logger
	//------------------------------------------
#if defined(ET_DEBUG)
#if defined(GRAPHICS_API_VERBOSE)

	auto glLogCallback = [](GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam)
	{
		UNUSED(source);
		UNUSED(id);
		UNUSED(length);
		UNUSED(userParam);

		LogLevel level = LogLevel::Info;
		switch (type)
		{
		case GL_DEBUG_TYPE_ERROR:
			level = LogLevel::Error;
			break;
		case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
			level = LogLevel::Warning;
			break;
		case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
			level = LogLevel::Warning;
			break;
		}

		if (severity == GL_DEBUG_SEVERITY_HIGH)
		{
			level = LogLevel::Error;
		}

		LOG(message, level);
		LOG("");
	}

	glEnable(GL_DEBUG_OUTPUT);
	glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
	glDebugMessageCallback(glLogCallback, nullptr);
	glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, NULL, true);

#endif
#endif

	WINDOW.WindowResizeEvent.AddListener(std::bind(&SdlRenderArea::OnResize, this));

	if (m_OnInit)
	{
		m_OnInit();
	}
}

//---------------------------------
// SdlRenderArea::Initialize
//
// Destroy the context and OpenGl window
//
void SdlRenderArea::Uninitialize()
{
	if (m_OnDeinit)
	{
		m_OnDeinit();
	}

	SDL_GL_DeleteContext(m_Context);
	m_Context = nullptr;

	SDL_DestroyWindow(m_Window);
	m_Window = nullptr;

	atexit(SDL_Quit);
}

//---------------------------------
// SdlRenderArea::Update
//
void SdlRenderArea::Update()
{
	if (m_ShouldDraw)
	{
		if (m_OnRender)
		{
			m_OnRender();
			SDL_GL_SwapWindow(m_Window); // swap render buffers
		}
	}
}

//---------------------------------
// SdlRenderArea::SetVSyncEnabled
//
void SdlRenderArea::SetVSyncEnabled(bool const val)
{
	SDL_GL_SetSwapInterval(val);
}

//---------------------------------
// SdlRenderArea::SetVSyncEnabled
//
void SdlRenderArea::OnResize()
{
	if (m_OnResize)
	{
		m_OnResize(etm::vecCast<float>(SETTINGS->Window.Dimensions));
	}
}

//---------------------------------
// SdlRenderArea::QueueDraw
//
// Make sure we draw on the next update
//
void SdlRenderArea::QueueDraw()
{
	m_ShouldDraw = true;
}

//---------------------------------
// SdlRenderArea::QueueDraw
//
// Make the graphics APIs context current
//
bool SdlRenderArea::MakeCurrent()
{
	int32 result = SDL_GL_MakeCurrent(m_Window, m_Context);

	if (result != 0)
	{
		LOG(FS("Setting current context with SDL failed (%i) > '%s'", result, SDL_GetError()), LogLevel::Error);
		return false;
	}

	return true;
}
