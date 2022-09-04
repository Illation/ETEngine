#pragma once
#include <EtRHI/GraphicsContext/Viewport.h>

#include <EtApplication/Rendering/GlfwRenderWindow.h>
#include <EtApplication/Rendering/BasicGuiRenderer.h>
#include <EtApplication/Core/GlfwEventBase.h>


namespace et {
namespace app {


class GuiApplication;


//-----------
// GuiWindow
//
// Window including proper UI functionality based on lower level GUI systems
//
class GuiWindow : public GlfwEventBase
{
	friend class GuiApplication;

	// construct destruct
	//--------------------
public:
	GuiWindow(core::WindowSettings const& settings);
	virtual ~GuiWindow();

private:
	void Init(); // called by the application
	void Deinit();

	// functionality
	//---------------
public:
	void SetGuiDocument(core::HashString const documentId);
	void StartDrag();

	// accessors
	//-----------
	GlfwRenderWindow& GetRenderWindow() { return m_RenderWindow; }
	gui::Context const& GetContext() { return m_GuiRenderer.GetContext(); }


	// Data
	///////

private:
	GlfwRenderWindow m_RenderWindow;
	rhi::Viewport m_Viewport;
	BasicGuiRenderer m_GuiRenderer;

	bool m_IsInitialized = false;
};


} // namespace app
} // namespace et
