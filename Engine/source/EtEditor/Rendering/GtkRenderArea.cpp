#include "stdafx.h"
#include "GtkRenderArea.h"

#include "EpoxyRenderDevice_OpenGL.h"
#include "GtkRenderWindow.h"

#include <EtRHI/GraphicsContext/ContextHolder.h>


namespace et {
namespace edit {


//=========================
// Single Context GL Area
//=========================


//---------------------------------
// SingleContextGlArea::c-tor
//
SingleContextGlArea::SingleContextGlArea()
	: Gtk::GLArea()
{
	signal_create_context().connect(sigc::mem_fun(*this, &SingleContextGlArea::OnCreateContext), false);
}

//---------------------------------
// SingleContextGlArea::c-tor
//
SingleContextGlArea::SingleContextGlArea(BaseObjectType* cobject, Glib::RefPtr<Gtk::Builder> const& refBuilder)
	: Gtk::GLArea(cobject)
{
	ET_UNUSED(refBuilder);
	signal_create_context().connect(sigc::mem_fun(*this, &SingleContextGlArea::OnCreateContext), false);
}

//---------------------------------
// SingleContextGlArea::OnCreateContext
//
Glib::RefPtr<Gdk::GLContext> SingleContextGlArea::OnCreateContext()
{
	rhi::GraphicsContext graphicsContext = rhi::ContextHolder::Instance().GetMainRenderContext();
	GtkRenderWindow* const renderWindow = static_cast<GtkRenderWindow*>(graphicsContext.GetSourceWindow());
	ET_ASSERT(renderWindow != nullptr, "Can't create GLarea before open GL context is initialized");

	set_has_depth_buffer(renderWindow->UseDepthBuffer());
	set_has_stencil_buffer(renderWindow->UseStencilBuffer());
	set_double_buffered(renderWindow->UseDoubleBuffering());

	return renderWindow->GetGdkContext();
}


//=====================
// GTK Render Area
//=====================


//---------------------------------
// GtkRenderArea::c-tor
//
// Create a Window and an openGL context to draw to the window
//
GtkRenderArea::GtkRenderArea(Ptr<SingleContextGlArea> const glArea)
	: I_RenderArea()
	, m_GlArea(glArea)
{

	m_GlArea->signal_realize().connect(sigc::mem_fun(*this, &GtkRenderArea::OnRealize));
	m_GlArea->signal_unrealize().connect(sigc::mem_fun(*this, &GtkRenderArea::OnUnrealize), false);
	m_GlArea->signal_resize().connect(sigc::mem_fun(*this, &GtkRenderArea::OnResize), false);
	m_GlArea->signal_render().connect(sigc::mem_fun(*this, &GtkRenderArea::OnRender), false);
}

//---------------------------------
// GtkRenderArea::OnRealize
//
void GtkRenderArea::OnRealize()
{
	if (m_OnInit)
	{
		m_OnInit(ToPtr(rhi::ContextHolder::GetRenderDevice()));
	}
}

//---------------------------------
// GtkRenderArea::OnUnrealize
//
void GtkRenderArea::OnUnrealize()
{
	if (m_OnDeinit)
	{
		m_OnDeinit();
	}
}

//---------------------------------
// GtkRenderArea::SetOnResize
//
void GtkRenderArea::OnResize(int32 x, int32 y)
{
	if (m_OnResize)
	{
		m_OnResize(math::vecCast<float>(ivec2(x, y)));
	}
}

//---------------------------------
// GtkRenderArea::SetOnRender
//
bool GtkRenderArea::OnRender(const Glib::RefPtr<Gdk::GLContext>& context)
{
	ET_UNUSED(context);

	if (m_OnRender)
	{
		m_OnRender(rhi::ContextHolder::GetRenderDevice()->GetActiveFramebuffer());
	}

	return true;
}

//---------------------------------
// GtkRenderArea::QueueDraw
//
// Make sure we draw on the next update
//
void GtkRenderArea::QueueDraw()
{
	m_GlArea->queue_draw();
}

//---------------------------------
// GtkRenderArea::QueueDraw
//
// Make the graphics APIs context current
//
bool GtkRenderArea::MakeCurrent()
{
	m_GlArea->make_current();
	try
	{
		m_GlArea->throw_if_error();
	}
	catch (const Gdk::GLError& gle)
	{
		ET_LOG_W(ET_CTX_EDITOR, "Viewport::MakeCurrent > An error occured making the context current during realize:");
		ET_LOG_W(ET_CTX_EDITOR, "%s-%s-%s", std::to_string(gle.domain()).c_str(), std::to_string(gle.code()).c_str(), gle.what().c_str());
		return false;
	}

	return true;
}

//---------------------------------
// GtkRenderArea::GetDimensions
//
ivec2 GtkRenderArea::GetDimensions() const
{
	ivec2 ret;

	ret.x = m_GlArea->get_allocated_width();
	ret.y = m_GlArea->get_allocated_height();

	return ret;
}


} // namespace edit
} // namespace et
