#include "stdafx.h"
#include "Viewport.h"

#include "ViewportRenderer.h"
#include "RenderArea.h"
#include "RenderState.h"


//=====================
// Viewport
//=====================


// static
Viewport* Viewport::g_CurrentViewport = nullptr;


//---------------------------------
// Viewport::Viewport
//
// Construct a viewport from a glArea
//
Viewport::Viewport(I_RenderArea* const area)
	: m_Area(area)
	, m_RenderState(new RenderState())
{
	RegisterAsTriggerer();

	m_Area->SetOnInit(std::function<void()>([this]() -> void { OnRealize(); }));
	m_Area->SetOnDeinit(std::function<void()>([this]() -> void { OnUnrealize(); }));
	m_Area->SetOnResize(std::function<void(vec2 const)>([this](vec2 const resolution) -> void { OnResize(resolution); }));
	m_Area->SetOnRender(std::function<void()>([this]() -> void { OnRender(); }));
}


//---------------------------------
// Viewport::Viewport
//
// Construct a viewport from a glArea
//
Viewport::~Viewport()
{
	if (m_Renderer != nullptr)
	{
		delete m_Renderer;
		m_Renderer = nullptr;
	}
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
// Viewport::GetGlobalRenderState
//
// returns the render state of the current viewport
//
RenderState* Viewport::GetGlobalRenderState()
{
	if (g_CurrentViewport == nullptr)
	{
		LOG("GetGlobalRenderState > g_CurrentViewport not set -> couldn't retrieve state", LogLevel::Error);
		return nullptr;
	}

	return g_CurrentViewport->GetState();
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
	m_RenderState->Initialize();

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

	if (!Render())
	{
		LOG("Something went wrong rendering!", LogLevel::Warning);
	}

	m_Area->QueueDraw(); // request drawing again
}

//---------------------------------
// Viewport::Render
//
// Draws the GL Area
//
bool Viewport::Render()
{
	if (m_Renderer != nullptr)
	{
		m_Renderer->OnRender();
	}
	else
	{
		// Draw pink to indicate that no renderer is attached
		STATE->SetClearColor(vec4(0.55f, 0.075f, 0.2f, 1.f));
		STATE->Clear(GL_COLOR_BUFFER_BIT);
	}

	STATE->Flush();

	return true;
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