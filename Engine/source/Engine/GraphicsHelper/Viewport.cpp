#include "stdafx.h"
#include "Viewport.h"

#include "ViewportRenderer.h"
#include "RenderArea.h"
#include "GraphicsApiContext.h"


//=====================
// Viewport
//=====================


// static
Viewport* Viewport::g_CurrentViewport = nullptr;


//---------------------------------
// Viewport::c-tor
//
// Construct a viewport from a glArea
//
Viewport::Viewport(I_RenderArea* const area)
	: m_Area(area)
	, m_ApiContext(new GraphicsApiContext())
{
	RegisterAsTriggerer();

	m_Area->SetOnInit(std::function<void()>(std::bind(&Viewport::OnRealize, this)));
	m_Area->SetOnDeinit(std::function<void()>(std::bind(&Viewport::OnUnrealize, this)));
	m_Area->SetOnResize(std::function<void(vec2 const)>(std::bind(&Viewport::OnResize, this, std::placeholders::_1)));
	m_Area->SetOnRender(std::function<void()>(std::bind(&Viewport::OnRender, this)));
}

//---------------------------------
// Viewport::d-tor
//
Viewport::~Viewport()
{
	SafeDelete(m_ApiContext);
}

//---------------------------------
// Viewport::Redraw
//
// Trigger redrawing the window
//
void Viewport::Redraw()
{
	m_Area->QueueDraw();
}

//---------------------------------
// Viewport::SetRenderer
//
// Set the renderer, takes ownership of it
//
void Viewport::SetRenderer(I_ViewportRenderer* renderer)
{
	m_Renderer = renderer;

	if (m_IsRealized)
	{
		OnRealize();
	}
}

//---------------------------------
// Viewport::GetCurrentApiContext
//
// returns the render state of the current viewport
//
GraphicsApiContext* Viewport::GetCurrentApiContext()
{
	ET_ASSERT(g_CurrentViewport != nullptr);
	return g_CurrentViewport->GetApiContext();
}

//---------------------------------
// Viewport::OnRealize
//
// init open gl stuff
//
void Viewport::OnRealize()
{
	MakeCurrent();

	// init render state
	m_ApiContext->Initialize();

	// init renderer
	if (m_Renderer != nullptr)
	{
		m_Renderer->OnResize(m_Dimensions);
		m_Renderer->OnInit();
	}

	m_IsRealized = true;
}

//---------------------------------
// Viewport::OnUnrealize
//
// Uninit open gl stuff
//
void Viewport::OnUnrealize()
{		
	MakeCurrent();

	if (m_Renderer != nullptr)
	{
		m_Renderer->OnDeinit();
	}

	m_IsRealized = false;
}

//---------------------------------
// Viewport::OnUnrealize
//
// When the drawable area resizes - we should recreate framebuffer textures here etc
//
void Viewport::OnResize(vec2 const resolution)
{
	MakeCurrent();

	m_Dimensions = etm::vecCast<int32>(resolution);

	if (m_Renderer != nullptr)
	{
		m_Renderer->OnResize(m_Dimensions);
	}
}

//---------------------------------
// Viewport::OnRender
//
// This function updates everything in a gameloops style and then calls Render, making sure to refresh itself at screen refresh rate
//
void Viewport::OnRender()
{
	MakeCurrent();

	TriggerTick(); // if this is the first real time thing we will start the update process here

	Render();

	m_Area->QueueDraw(); // request drawing again
}

//---------------------------------
// Viewport::Render
//
// Draws the GL Area
//
void Viewport::Render()
{
	GraphicsApiContext* const api = Viewport::GetCurrentApiContext();

	if (m_Renderer != nullptr)
	{
		m_Renderer->OnRender();
	}
	else
	{
		// Draw pink to indicate that no renderer is attached
		api->SetClearColor(vec4(0.55f, 0.075f, 0.2f, 1.f));
		api->Clear(GL_COLOR_BUFFER_BIT);
	}

	api->Flush();
}

//---------------------------------
// Viewport::MakeCurrent
//
// Makes this the active viewport that openGl draws to
//
void Viewport::MakeCurrent()
{
	if (!(m_Area->MakeCurrent()))
	{
		LOG("Viewport::MakeCurrent > An error occured making the context current during realize!", LogLevel::Warning);
	}

	g_CurrentViewport = this;
}