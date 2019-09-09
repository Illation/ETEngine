#pragma once
#include <SDL.h>

#include "GraphicsHelper/RenderArea.h"


//---------------------------------
// SdlRenderArea
//
// Render area using SDL windows as a subsystem, managing an openGL context
//
class SdlRenderArea : public I_RenderArea
{
	// construct destruct
	//-------------------
public:
	SdlRenderArea() : I_RenderArea() {}
	virtual ~SdlRenderArea() = default;

	void Initialize();
	void Uninitialize();

	void Update();

	void SetVSyncEnabled(bool const val);

private:
	void OnResize();

	// Render Area Interface
	//-----------------------
protected:
	void SetOnInit(std::function<void()>& callback) override { m_OnInit = callback; }
	void SetOnDeinit(std::function<void()>& callback) override { m_OnDeinit = callback; }
	void SetOnResize(std::function<void(vec2 const)>& callback) override { m_OnResize = callback; }
	void SetOnRender(std::function<void()>& callback) override { m_OnRender = callback; }

	void QueueDraw() override;
	bool MakeCurrent() override;

	// Data
	///////
private:
	std::function<void()> m_OnInit;
	std::function<void()> m_OnDeinit;
	std::function<void(vec2 const)> m_OnResize;
	std::function<void()> m_OnRender;

	SDL_GLContext m_Context = nullptr;
	SDL_Window* m_Window = nullptr;

	bool m_ShouldDraw = false;
};

