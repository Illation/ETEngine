#pragma once

#include <EtCore/UpdateCycle/RealTimeTickTriggerer.h>


class RenderState;
class I_ViewportRenderer;
class I_RenderArea;


//---------------------------------
// Viewport
//
// Contains a drawable area
//  - #todo support non realtime rendering
//
class Viewport : public I_RealTimeTickTriggerer
{
public:
	Viewport(I_RenderArea* const area);
	virtual ~Viewport();

	void Redraw();

	void SetRenderer(I_ViewportRenderer* renderer); // takes ownership

	RenderState* GetState() { return m_RenderState; }

	static RenderState* GetGlobalRenderState();

protected:

	void OnRealize();
	void OnUnrealize();
	void OnResize(vec2 const resolution);
	void OnRender();

private:
	bool Render();

	void MakeCurrent();


	// Data
	///////

private:
	static Viewport* g_CurrentViewport;


	I_RenderArea* m_Area = nullptr;

	I_ViewportRenderer* m_Renderer = nullptr;
	RenderState* m_RenderState = nullptr; // since a viewport has it's own open gl context, each viewport has it's own render state

	ivec2 m_Dimensions;

	bool m_IsRealized = false;
};
