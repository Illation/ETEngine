#include "stdafx.h"
#include "GlfwRenderArea.h"

#include <glad/glad.h>


//=====================
// GLFW Render Area
//=====================


//---------------------------------
// GlfwRenderArea::Initialize
//
// Create a Window and an openGL context to draw to the window
//
void GlfwRenderArea::Initialize()
{
	// Initialize GLFW 
	//-----------------
	if (!glfwInit())
	{
		LOG("Couldn't initialize GLFW!", LogLevel::Error);
	}

	// error callback
	glfwSetErrorCallback([](int32 const code, char const* const description)
		{
			LOG(FS("GLFW error [%i]: %s", code, description), LogLevel::Warning);
		});

	// Create Window and openGL context in one place
	//-----------------------------------------------

	// Set the OopenGL context attributes before creating the window
	glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_API);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
	// enable double buffering
	glfwWindowHint(GLFW_DOUBLEBUFFER, GLFW_TRUE);
	// Also request a depth buffer
	glfwWindowHint(GLFW_DEPTH_BITS, 24);

#if defined(GRAPHICS_API_DEBUG)
	// Request a debug context.
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);
#endif

	Settings* const settings = Settings::GetInstance();

	GLFWmonitor* const primaryMonitor = glfwGetPrimaryMonitor();
	GLFWmonitor* fullscreenMonitor = nullptr;
	if (settings->Window.Fullscreen)
	{
		fullscreenMonitor = primaryMonitor;
	}

#ifdef EDITOR
	ivec2 const dim = settings->Window.EditorDimensions;
#else
	ivec2 const dim = settings->Window.Dimensions;
#endif

	m_Window = glfwCreateWindow(dim.x, dim.y, settings->Window.Title.c_str(), fullscreenMonitor, nullptr);
	if (m_Window == nullptr)
	{
		glfwTerminate();
		LOG("Failed to create window with GLFW!", LogLevel::Error);
	}

	// Check OpenGL properties and create open gl function pointers
	//--------------------------------------------------------------
	MakeCurrent();
	gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
	SetVSyncEnabled(true);

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

	WINDOW.WindowResizeEvent.AddListener(std::bind(&GlfwRenderArea::OnResize, this));

	if (m_OnInit)
	{
		m_OnInit();
	}
}

//---------------------------------
// GlfwRenderArea::Initialize
//
// Destroy the context and OpenGl window
//
void GlfwRenderArea::Uninitialize()
{
	if (m_OnDeinit)
	{
		m_OnDeinit();
	}

	glfwDestroyWindow(m_Window);
	m_Window = nullptr;

	glfwTerminate();
}

//---------------------------------
// GlfwRenderArea::Update
//
void GlfwRenderArea::Update()
{
	if (m_ShouldDraw)
	{
		if (m_OnRender)
		{
			m_OnRender();
			glfwSwapBuffers(m_Window); // swap render buffers
		}
	}
}

//---------------------------------
// GlfwRenderArea::SetVSyncEnabled
//
void GlfwRenderArea::SetVSyncEnabled(bool const val)
{
	glfwSwapInterval(val);
}

//---------------------------------
// GlfwRenderArea::SetVSyncEnabled
//
void GlfwRenderArea::OnResize()
{
	if (m_OnResize)
	{
		m_OnResize(etm::vecCast<float>(SETTINGS->Window.Dimensions));
	}
}

//---------------------------------
// GlfwRenderArea::QueueDraw
//
// Make sure we draw on the next update
//
void GlfwRenderArea::QueueDraw()
{
	m_ShouldDraw = true;
}

//---------------------------------
// GlfwRenderArea::QueueDraw
//
// Make the graphics APIs context current
//
bool GlfwRenderArea::MakeCurrent()
{
	glfwMakeContextCurrent(m_Window);
	return true;
}
