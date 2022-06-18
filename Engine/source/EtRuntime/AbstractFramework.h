#pragma once
#include <EtCore/UpdateCycle/RealTimeTickTriggerer.h>
#include <EtCore/UpdateCycle/Tickable.h>

#include <EtGUI/Rendering/SceneRendererGUI.h>

#include <EtFramework/Config/TickOrder.h>
#include <EtFramework/Util/ScreenshotCapture.h>

#include <EtRuntime/Rendering/GlfwRenderWindow.h>


// fwd
namespace et { namespace render {
	class Viewport;
	class ShadedSceneRenderer;
	class SplashScreenRenderer;
} }


namespace et {
namespace rt {


//----------------------------
// AbstractFramework
//
// Ties various engine systems together, loads them in the correct order, and provides an interface for Applications to instantiate and extend
//  - To create a new project, create a class inheriting from this and run it
//
class AbstractFramework : public core::I_RealTimeTickTriggerer, public core::I_Tickable
{
	// construct destruct
	//--------------------
public:
	AbstractFramework() : core::I_Tickable(static_cast<uint32>(fw::E_TickOrder::TICK_Framework)) {}
	virtual ~AbstractFramework();

	// accessors
	//-----------
	std::vector<core::HashString> const& GetSceneIds() const { return m_Scenes; }

	// functionality
	//---------------
	void Run();

	// interface
	//-----------
protected:
	virtual void OnSystemInit() = 0;
	virtual void OnInit() = 0;
	virtual void OnTick() override = 0;

	// utility
	//---------
private:
	void MainLoop();

	// Data
	///////

private:
	GlfwRenderWindow m_RenderWindow;
	UniquePtr<render::Viewport> m_Viewport;

	UniquePtr<render::ShadedSceneRenderer> m_SceneRenderer;
	UniquePtr<rt::SplashScreenRenderer> m_SplashScreenRenderer;

protected:
	gui::SceneRendererGUI m_GuiRenderer;
private:
	gui::T_ContextId m_GuiContext = gui::INVALID_CONTEXT_ID;


	std::vector<core::HashString> m_Scenes;

protected:
	fw::ScreenshotCapture m_ScreenshotCapture;
};


} // namespace rt
} // namespace et
