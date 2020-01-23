#include "stdafx.h"
#include "GtkRenderArea.h"

#include "EpoxyGlContext.h"


namespace et {
namespace edit {


//=========================
// Single Context GL Area
//=========================


// static
SingleContextGlArea::ContextCache* SingleContextGlArea::s_SingleContextCache = nullptr;


//---------------------------------
// SingleContextGlArea::DestroyContext
//
void SingleContextGlArea::DestroyContext()
{
	delete s_SingleContextCache;
	s_SingleContextCache = nullptr;
}

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
	UNUSED(refBuilder);
	signal_create_context().connect(sigc::mem_fun(*this, &SingleContextGlArea::OnCreateContext), false);
}

//---------------------------------
// SingleContextGlArea::OnCreateContext
//
Glib::RefPtr<Gdk::GLContext> SingleContextGlArea::OnCreateContext()
{
	if (s_SingleContextCache == nullptr)
	{
		s_SingleContextCache = new ContextCache();
		s_SingleContextCache->glibContext = Gtk::GLArea::on_create_context();
		s_SingleContextCache->apiContext = new render::EpoxyGlContext();
	}

	return s_SingleContextCache->glibContext;
}

//---------------------------------
// SingleContextGlArea::GetApiContext
//
render::I_GraphicsApiContext* SingleContextGlArea::GetApiContext() const
{
	ET_ASSERT(s_SingleContextCache != nullptr);
	ET_ASSERT(s_SingleContextCache->apiContext != nullptr);

	return s_SingleContextCache->apiContext;
}


//=====================
// GTK Render Area
//=====================


//---------------------------------
// GtkRenderArea::c-tor
//
// Create a Window and an openGL context to draw to the window
//
GtkRenderArea::GtkRenderArea(SingleContextGlArea* const glArea)
	: I_RenderArea()
	, m_GlArea(glArea)
{
	m_GlArea->set_has_depth_buffer(true);
	m_GlArea->set_has_stencil_buffer(false);
	m_GlArea->set_required_version(4, 5);
	m_GlArea->set_double_buffered(true);
	m_GlArea->set_use_es(false);

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
		m_OnInit(m_GlArea->GetApiContext());
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
		m_OnResize(etm::vecCast<float>(ivec2(x, y)));
	}
}

//---------------------------------
// GtkRenderArea::SetOnRender
//
bool GtkRenderArea::OnRender(const Glib::RefPtr<Gdk::GLContext>& context)
{
	UNUSED(context);

	if (m_OnRender)
	{
		m_OnRender(m_GlArea->GetApiContext()->GetActiveFramebuffer());
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
		LOG("Viewport::MakeCurrent > An error occured making the context current during realize:", core::LogLevel::Warning);
		LOG(std::to_string(gle.domain()) + std::string("-") + std::to_string(gle.code()) + std::string("-") + gle.what().raw(), core::LogLevel::Warning);
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
