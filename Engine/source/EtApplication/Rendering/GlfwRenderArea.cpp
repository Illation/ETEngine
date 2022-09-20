#include "stdafx.h"
#include "GlfwRenderArea.h"

#include "GladRenderDevice_OpenGL.h"


namespace et {
namespace app {


//=====================
// GLFW Render Area
//=====================


// static
ivec2 const GlfwRenderArea::s_DefaultDimensions(512, 512);


//---------------------------------
// GlfwRenderArea::Initialize
//
// Create a Window and an openGL context to draw to the window
//
void GlfwRenderArea::Initialize(rhi::RenderDeviceParams const& params, bool const hidden, core::WindowSettings const& windowSettings)
{
	// Initialize GLFW 
	//-----------------
	if (!glfwInit())
	{
		ET_LOG_E(ET_CTX_APP, "Couldn't initialize GLFW!");
	}

	// error callback
	glfwSetErrorCallback([](int32 const code, char const* const description)
		{
			ET_TRACE_W(ET_CTX_APP, "GLFW error [%i]: %s", code, description);
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

	// other window info
	glfwWindowHint(GLFW_DECORATED, windowSettings.m_Decorated ? GLFW_TRUE : GLFW_FALSE);

	// Determine window settings
	GLFWmonitor* fullscreenMonitor = nullptr;
	ivec2 dim;
	std::string title;
	
	if (hidden)
	{
		glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);

		dim = s_DefaultDimensions;
	}
	else
	{
		dim = windowSettings.GetSize();
		title = windowSettings.m_Title;

		if (windowSettings.m_Fullscreen)
		{
			GLFWmonitor* const primaryMonitor = glfwGetPrimaryMonitor();
			fullscreenMonitor = primaryMonitor;
		}
	}

	// Window creation
	m_Window = ToPtr(glfwCreateWindow(dim.x, dim.y, title.c_str(), fullscreenMonitor, nullptr));
	if (m_Window == nullptr)
	{
		glfwTerminate();
		ET_LOG_E(ET_CTX_APP, "Failed to create window with GLFW!");
	}

	// Check OpenGL properties and create open gl function pointers
	//--------------------------------------------------------------
	MakeCurrent();
	gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
	SetVSyncEnabled(true);

	ET_ASSERT(m_RenderDevice == nullptr);
	m_RenderDevice = Create<rhi::GladGlDevice>();
	m_RenderDevice->Initialize(dim);

	if (m_OnInit)
	{
		m_OnInit(m_RenderDevice);
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

	m_RenderDevice = nullptr;

	glfwDestroyWindow(m_Window.Get());
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
			glfwSwapBuffers(m_Window.Get()); // swap render buffers
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
	glfwMakeContextCurrent(m_Window.Get());
	return true;
}

//---------------------------------
// GtkRenderArea::GetDimensions
//
ivec2 GlfwRenderArea::GetDimensions() const
{
	ivec2 ret;

	glfwGetWindowSize(m_Window.Get(), &ret.x, &ret.y);

	return ret;
}


} // namespace app
} // namespace et
