#include "stdafx.h"
#include "GlfwRenderArea.h"
#include "GladGlContext.h"


namespace et {
namespace rt {


//=====================
// GLFW Render Area
//=====================


//---------------------------------
// GlfwRenderArea::Initialize
//
// Create a Window and an openGL context to draw to the window
//
void GlfwRenderArea::Initialize(render::GraphicsContextParams const& params)
{
	// Initialize GLFW 
	//-----------------
	if (!glfwInit())
	{
		LOG("Couldn't initialize GLFW!", core::LogLevel::Error);
	}

	// error callback
	glfwSetErrorCallback([](int32 const code, char const* const description)
		{
			LOG(FS("GLFW error [%i]: %s", code, description), core::LogLevel::Warning);
		});

	// Create Window and openGL context in one place
	//-----------------------------------------------

	// Set the OopenGL context attributes before creating the window
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, params.m_VersionMajor);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, params.m_VersionMinor);
	glfwWindowHint(GLFW_OPENGL_PROFILE, params.m_IsForwardCompatible ? GLFW_OPENGL_CORE_PROFILE : GLFW_OPENGL_COMPAT_PROFILE);
	glfwWindowHint(GLFW_CLIENT_API, params.m_UseES ? GLFW_OPENGL_ES_API : GLFW_OPENGL_API);

	// Request a debug context.
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, params.m_UseDebugInfo ? GLFW_TRUE : GLFW_FALSE);

	// buffer info
	glfwWindowHint(GLFW_DOUBLEBUFFER, params.m_DoubleBuffer ? GLFW_TRUE : GLFW_FALSE);
	glfwWindowHint(GLFW_DEPTH_BITS, params.m_DepthBuffer ? 24 : 0);
	glfwWindowHint(GLFW_STENCIL_BITS, params.m_StencilBuffer ? 8 : 0);

	fw::Config::Settings::Window const& windowSettings = fw::Config::GetInstance()->GetWindow();

	GLFWmonitor* const primaryMonitor = glfwGetPrimaryMonitor();
	GLFWmonitor* fullscreenMonitor = nullptr;
	if (windowSettings.Fullscreen)
	{
		fullscreenMonitor = primaryMonitor;
	}

	ivec2 const dim = windowSettings.GetSize();

	m_Window = glfwCreateWindow(dim.x, dim.y, windowSettings.Title.c_str(), fullscreenMonitor, nullptr);
	if (m_Window == nullptr)
	{
		glfwTerminate();
		LOG("Failed to create window with GLFW!", core::LogLevel::Error);
	}

	glfwSetWindowUserPointer(m_Window, this);

	// Check OpenGL properties and create open gl function pointers
	//--------------------------------------------------------------
	MakeCurrent();
	gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
	SetVSyncEnabled(true);

	if (m_OnInit)
	{
		ET_ASSERT(m_Context == nullptr);
		m_Context = new render::GladGlContext();

		m_OnInit(m_Context);
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

	delete m_Context;

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
			m_OnRender(0u);
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
void GlfwRenderArea::SetSize(ivec2 const size)
{
	if (m_OnResize)
	{
		m_OnResize(math::vecCast<float>(size));
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

//---------------------------------
// GtkRenderArea::GetDimensions
//
ivec2 GlfwRenderArea::GetDimensions() const
{
	ivec2 ret;

	glfwGetWindowSize(m_Window, &ret.x, &ret.y);

	return ret;
}


} // namespace rt
} // namespace et
