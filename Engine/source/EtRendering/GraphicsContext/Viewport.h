#pragma once
#include <EtCore/UpdateCycle/RealTimeTickTriggerer.h>

#include "ViewportEvents.h"


namespace et {
namespace render {


class I_GraphicsContextApi;
class I_ViewportRenderer;
class I_RenderArea;


//---------------------------------
// I_ViewportListener
//
// Interface that can react to viewport events
//
class I_ViewportListener
{
public:

	virtual ~I_ViewportListener() = default;

	virtual void OnViewportPreRender(T_FbLoc const targetFb) = 0;
	virtual void OnViewportPostFlush(T_FbLoc const targetFb) = 0;
};


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
private:
	void Render(T_FbLoc const targetFb);

public:
	void RegisterListener(I_ViewportListener* const listener);
	void UnregisterListener(I_ViewportListener* const listener);

	// accessors
	//-----------
	I_ViewportRenderer* GetViewportRenderer() { return m_Renderer; }
	I_GraphicsContextApi* GetApiContext() { return m_ApiContext; }

	ivec2 GetDimensions() const { return m_Dimensions; }
	float GetAspectRatio() const { return m_AspectRatio; }
	render::T_ViewportEventDispatcher& GetEventDispatcher() { return m_Events; }

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

	std::vector<I_ViewportListener*> m_Listeners; // #todo: use the event dispatcher instead
	render::T_ViewportEventDispatcher m_Events;
};


} // namespace render
} // namespace et
