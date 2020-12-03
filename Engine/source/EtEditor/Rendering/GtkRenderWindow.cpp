#include "stdafx.h"
#include "GtkRenderWindow.h"

#include "EpoxyGlContext.h"


namespace et {
namespace edit {


//====================
// GTK Render Window
//====================


//---------------------------------
// GtkRenderWindow::d-tor
//
GtkRenderWindow::~GtkRenderWindow()
{
	delete m_Context;
}

//---------------------------------
// GtkRenderWindow::CreateContext
//
render::I_GraphicsContextApi* GtkRenderWindow::CreateContext(render::GraphicsContextParams const& params)
{
	ET_ASSERT(m_Source != nullptr, "Source window must be set before creating context");
	ET_ASSERT(m_Source->get_realized(), "Source window must be realized before creating context");

	Glib::RefPtr<Gdk::Window> const gdkWindow = m_Source->get_window();
	ET_ASSERT(gdkWindow ? true : false); // can't implicitly cast to bool apparently

	try
	{
		m_GdkContext = gdkWindow->create_gl_context();
	}
	catch (Gdk::GLError const& ex)
	{
		ET_ASSERT(false, " > create > %s", ex.what().c_str());
		return nullptr;
	}
	catch (std::exception const& ex)
	{
		ET_ASSERT(false, " > create > %s", ex.what());
		return nullptr;
	}

	ET_ASSERT(m_GdkContext ? true : false);

	m_GdkContext->set_required_version(params.m_VersionMajor, params.m_VersionMinor);
	m_GdkContext->set_use_es(params.m_UseES);
	m_GdkContext->set_forward_compatible(params.m_IsForwardCompatible);
	m_GdkContext->set_debug_enabled(params.m_UseDebugInfo);

	// save other params for later draw area creation
	m_UseDoubleBuffering = params.m_DoubleBuffer;
	m_UseDepthBuffer = params.m_DepthBuffer;
	m_UseStencilBuffer = params.m_StencilBuffer;

	try
	{
		m_GdkContext->realize();
	}
	catch (Gdk::GLError const& ex)
	{
		ET_ASSERT(false, " > realize > %s", ex.what().c_str());
		return nullptr;
	}
	catch (std::exception const& ex)
	{
		ET_ASSERT(false, " > realize > %s", ex.what());
		return nullptr;
	}

	m_GdkContext->make_current(); // not calling this function can apparently kill a computer...

	LOG("\nGDK GLContext settings:");

	int major, minor;
	m_GdkContext->get_version(major, minor);
	LOG(FS("\tversion %i %i", major, minor));
	LOG(FS("\tis_legacy %s", m_GdkContext->is_legacy() ? "true" : "false"));
	LOG(FS("\tuse_es %s", m_GdkContext->get_use_es() ? "true" : "false"));
	LOG(FS("\tforward_compatible %s", m_GdkContext->get_forward_compatible() ? "true" : "false"));
	LOG(FS("\tdebug_enabled %s", m_GdkContext->get_debug_enabled() ? "true" : "false"));

	LOG("");

	m_Context = new render::EpoxyGlContext();

	ivec2 dim;
	dim.x = m_Source->get_allocated_width();
	dim.y = m_Source->get_allocated_height();

	m_Context->Initialize(dim);

	return m_Context;
}


} // namespace edit
} // namespace et

