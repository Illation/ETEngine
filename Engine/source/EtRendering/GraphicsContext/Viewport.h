#pragma once
#include <EtCore/UpdateCycle/RealTimeTickTriggerer.h>

#include "ViewportEvents.h"


// fwd
namespace et { namespace core {
	class RawInputProvider;
} }


namespace et {
namespace render {


class I_GraphicsContextApi;
class I_ViewportRenderer;
class I_RenderArea;


//---------------------------------
// Viewport
//
// Contains a drawable area
//  - #todo support non realtime rendering
//
class Viewport final : public core::I_RealTimeTickTriggerer
{
	// definitions
	//---------------
	static Viewport* g_CurrentViewport;

public:
	static Viewport* GetCurrentViewport();

	// construct destruct
	//-------------------
	Viewport(I_RenderArea* const area);
	~Viewport();

	// functionality
	//---------------
	void Redraw();
	void SynchDimensions();
	void SetRenderer(I_ViewportRenderer* renderer);
	void SetActive(bool const val) { m_IsActive = val; }
	void SetTickDisabled(bool const val) { m_TickDisabled = val; }
	void SetInputProvider(Ptr<core::RawInputProvider> const input) { m_InputProvider = input; }
private:
	void Render(T_FbLoc const targetFb);

	// accessors
	//-----------
public:
	I_ViewportRenderer* GetViewportRenderer() { return m_Renderer; }
	I_GraphicsContextApi* GetApiContext() { return m_ApiContext; }

	ivec2 GetDimensions() const { return m_Dimensions; }
	float GetAspectRatio() const { return m_AspectRatio; }

	T_ViewportEventDispatcher& GetEventDispatcher() { return m_Events; }

	core::RawInputProvider* GetInputProvider() { return m_InputProvider.Get(); }

	// callbacks
	//-----------
protected:
	void OnRealize(I_GraphicsContextApi* const api);
	void OnUnrealize();
	void OnResize(vec2 const resolution);
	void OnRender(T_FbLoc const targetFb);

	// utility
	//-----------
public:
	void MakeCurrent();


	// Data
	///////
private:

	I_RenderArea* m_Area = nullptr;
	I_ViewportRenderer* m_Renderer = nullptr;

	I_GraphicsContextApi* m_ApiContext = nullptr; 

	ivec2 m_Dimensions;
	float m_AspectRatio;

	bool m_IsRealized = false;
	bool m_IsActive = true;

	bool m_TickDisabled = false;

	T_ViewportEventDispatcher m_Events;

	Ptr<core::RawInputProvider> m_InputProvider;
};


} // namespace render
} // namespace et
