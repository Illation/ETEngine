#pragma once
#include <EtCore/UpdateCycle/RealTimeTickTriggerer.h>
#include <EtCore/Helper/MulticastDelegate.h>


class I_GraphicsApiContext;
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
class Viewport final : public I_RealTimeTickTriggerer
{
	// definitions
	//---------------
	static Viewport* g_CurrentViewport;

public:
	static I_GraphicsApiContext* GetCurrentApiContext();
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
private:
	void Render(T_FbLoc const targetFb);

public:
	void RegisterListener(I_ViewportListener* const listener);
	void UnregisterListener(I_ViewportListener* const listener);

	// accessors
	//-----------
	I_ViewportRenderer* GetViewportRenderer() { return m_Renderer; }
	I_GraphicsApiContext* GetApiContext() { return m_ApiContext; }

	ivec2 GetDimensions() const { return m_Dimensions; }
	float GetAspectRatio() const { return m_AspectRatio; }
	MulticastDelegate& GetResizeEvent() { return m_ResizeEvent; }

	// callbacks
	//-----------
protected:
	void OnRealize(I_GraphicsApiContext* const api);
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
	I_GraphicsApiContext* m_ApiContext = nullptr; 

	ivec2 m_Dimensions;
	float m_AspectRatio;
	MulticastDelegate m_ResizeEvent;

	bool m_IsRealized = false;
	bool m_IsActive = true;

	std::vector<I_ViewportListener*> m_Listeners;
};
