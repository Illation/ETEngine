#include "stdafx.h"
#include "GtkRenderArea.h"

#include "EpoxyGlContext.h"


//=====================
// GTK Render Area
//=====================


//---------------------------------
// GtkRenderArea::c-tor
//
// Create a Window and an openGL context to draw to the window
//
GtkRenderArea::GtkRenderArea(Gtk::GLArea* glArea)
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
		m_OnInit(new EpoxyGlContext());
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
		m_OnRender();
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
		LOG("Viewport::MakeCurrent > An error occured making the context current during realize:", LogLevel::Warning);
		LOG(std::to_string(gle.domain()) + std::string("-") + std::to_string(gle.code()) + std::string("-") + gle.what().raw(), LogLevel::Warning);
		return false;
	}

	return true;
}
