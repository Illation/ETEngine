#pragma once
#include <iostream>

#include <EtCore/UpdateCycle/RealTimeTickTriggerer.h>
#include <EtCore/UpdateCycle/Tickable.h>

#include <EtFramework/Config/TickOrder.h>
#include <EtFramework/Util/ScreenshotCapture.h>

#include <EtRuntime/Rendering/GlfwRenderArea.h>


class Viewport;
namespace render {
	class ShadedSceneRenderer;
	class SplashScreenRenderer;
}


class AbstractFramework : public I_RealTimeTickTriggerer, public I_Tickable
{
public:
	AbstractFramework() : I_Tickable(static_cast<uint32>(E_TickOrder::TICK_Framework)) {}
	virtual ~AbstractFramework();

	void Run();

protected:
	virtual void OnSystemInit() = 0;
	virtual void OnInit() = 0;
	virtual void OnTick() override = 0;

private:
	void MainLoop();

private:
	GlfwRenderArea m_RenderArea;
	Viewport* m_Viewport = nullptr;
	render::ShadedSceneRenderer* m_SceneRenderer = nullptr;
	render::SplashScreenRenderer* m_SplashScreenRenderer = nullptr;

protected:
	ScreenshotCapture m_ScreenshotCapture;
};
