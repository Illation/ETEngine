#include "stdafx.h"
#include "GtkRenderArea.h"

#include <Engine/GraphicsHelper/GladGlContext.h>


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

}

void GtkRenderArea::SetOnInit(std::function<void(I_GraphicsApiContext* const)>& callback)
{

}

void GtkRenderArea::SetOnDeinit(std::function<void()>& callback)
{

}

void GtkRenderArea::SetOnResize(std::function<void(vec2 const)>& callback)
{

}

void GtkRenderArea::SetOnRender(std::function<void()>& callback)
{

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
