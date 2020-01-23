#include "stdafx.h"
#include "Viewport.h"

#include "ViewportRenderer.h"
#include "RenderArea.h"


namespace et {
namespace render {


//=====================
// Viewport
//=====================


// static
Viewport* Viewport::g_CurrentViewport = nullptr;

//---------------------------------
// Viewport::GetCurrentApiContext
//
// returns the render state of the current viewport
//
I_GraphicsApiContext* Viewport::GetCurrentApiContext()
{
	ET_ASSERT(g_CurrentViewport != nullptr);
	return g_CurrentViewport->GetApiContext();
}

//---------------------------------
// Viewport::GetCurrentViewport
//
Viewport* Viewport::GetCurrentViewport()
{
	return g_CurrentViewport;
}


//-------------------------------------------------------------------------------


//---------------------------------
// Viewport::c-tor
//
// Construct a viewport from a glArea
//
Viewport::Viewport(I_RenderArea* const area)
	: m_Area(area)
{
	RegisterAsTriggerer();

	m_Area->SetOnInit(std::function<void(I_GraphicsApiContext* const)>(std::bind(&Viewport::OnRealize, this, std::placeholders::_1)));
	m_Area->SetOnDeinit(std::function<void()>(std::bind(&Viewport::OnUnrealize, this)));
	m_Area->SetOnResize(std::function<void(vec2 const)>(std::bind(&Viewport::OnResize, this, std::placeholders::_1)));
	m_Area->SetOnRender(std::function<void(T_FbLoc const)>(std::bind(&Viewport::OnRender, this, std::placeholders::_1)));
}

//---------------------------------
// Viewport::d-tor
//
Viewport::~Viewport()
{
	if (g_CurrentViewport == this)
	{
		g_CurrentViewport = nullptr;
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
// Viewport::SynchDimensions
//
// Ensure the dimensions are in line with the render areas size - can be useful during application bootup
//
void Viewport::SynchDimensions()
{
	OnResize(etm::vecCast<float>(m_Area->GetDimensions()));
}

//---------------------------------
// Viewport::SetRenderer
//
// Set the renderer, takes ownership of it
//
void Viewport::SetRenderer(I_ViewportRenderer* renderer)
{
	m_Renderer = renderer;

	if (m_Renderer == nullptr) // for deinitialization
	{
		return;
	}

	if (m_IsRealized)
	{
		OnRealize(m_ApiContext);
	}
}

//---------------------------------
// Viewport::OnRealize
//
// From this point on graphics API functions can be called for this viewport
//
void Viewport::OnRealize(I_GraphicsApiContext* const api)
{
	m_ApiContext = api;

	MakeCurrent();

	m_Dimensions = m_Area->GetDimensions();

	// init render state
	m_ApiContext->Initialize(m_Dimensions);

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
	m_AspectRatio = resolution.x / resolution.y;

	if (m_Renderer != nullptr)
	{
		m_Renderer->OnResize(m_Dimensions);
	}

	m_Events.Notify(render::E_ViewportEvent::VP_Resized, new render::ViewportEventData(this, m_Dimensions));
}

//---------------------------------
// Viewport::OnRender
//
// This function updates everything in a gameloops style and then calls Render, making sure to refresh itself at screen refresh rate
//
void Viewport::OnRender(T_FbLoc const targetFb)
{
	if (!m_TickDisabled)
	{
		TriggerTick(); // if this is the first real time thing we will start the update process here
	}

	if (m_IsActive)
	{
		MakeCurrent();
		Render(targetFb);

		m_Area->QueueDraw(); // request drawing again
	}
}

//---------------------------------
// Viewport::Render
//
// Draws the GL Area
//
void Viewport::Render(T_FbLoc const targetFb)
{
	if (m_Renderer != nullptr)
	{
		for (I_ViewportListener* const listener : m_Listeners)
		{
			listener->OnViewportPreRender(targetFb);
		}

		m_Renderer->OnRender(targetFb);
	}
	else
	{
		// Draw pink to indicate that no renderer is attached
		m_ApiContext->SetClearColor(vec4(0.55f, 0.075f, 0.2f, 1.f));
		m_ApiContext->Clear(E_ClearFlag::Color);
	}

	m_ApiContext->Flush();

	for (I_ViewportListener* const listener : m_Listeners)
	{
		listener->OnViewportPostFlush(targetFb);
	}
}

//---------------------------------
// Viewport::RegisterListener
//
void Viewport::RegisterListener(I_ViewportListener* const listener)
{
	ET_ASSERT(std::find(m_Listeners.cbegin(), m_Listeners.cend(), listener) == m_Listeners.cend(), "Listener already registered!");

	m_Listeners.emplace_back(listener);
}

//---------------------------------
// Viewport::UnregisterListener
//
void Viewport::UnregisterListener(I_ViewportListener* const listener)
{
	// try finding the listener
	auto listenerIt = std::find(m_Listeners.begin(), m_Listeners.end(), listener);

	// it should have been found
	if (listenerIt == m_Listeners.cend())
	{
		LOG("Viewport::UnregisterListener > Listener not found", core::LogLevel::Warning);
		return;
	}

	// swap and remove - the order of the listener list doesn't matter
	if (m_Listeners.size() > 1u)
	{
		std::iter_swap(listenerIt, std::prev(m_Listeners.end()));
		m_Listeners.pop_back();
	}
	else
	{
		m_Listeners.clear();
	}
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
		LOG("Viewport::MakeCurrent > An error occured making the context current during realize!", core::LogLevel::Warning);
	}

	g_CurrentViewport = this;
}

} // namespace render
} // namespace et
