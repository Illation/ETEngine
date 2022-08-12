#pragma once

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <EtCore/Util/WindowSettings.h>

#include <EtRHI/GraphicsContext/RenderArea.h>


namespace et {
namespace app {


//---------------------------------
// GlfwRenderArea
//
// Render area using GLFW windows as a subsystem, managing an openGL context
//
class GlfwRenderArea final : public rhi::I_RenderArea
{
	static ivec2 const s_DefaultDimensions;

	// construct destruct
	//-------------------
public:
	GlfwRenderArea() : rhi::I_RenderArea() {}
	virtual ~GlfwRenderArea() = default;

	void Initialize(rhi::RenderDeviceParams const& params, bool const hidden, core::WindowSettings const& windowSettings);
	void Uninitialize();

	// functionality
	//---------------
	void Update();
	void SetVSyncEnabled(bool const val);

	void SetSize(ivec2 const size);

	// accessors
	//-----------
	GLFWwindow* GetWindow() const { return m_Window.Get(); }
	rhi::I_RenderDevice* GetRenderDevice() const { return m_RenderDevice.Get(); }

	// Render Area Interface
	//-----------------------
protected:
	void SetOnInit(std::function<void(Ptr<rhi::I_RenderDevice> const)>& callback) override { m_OnInit = callback; }
	void SetOnDeinit(std::function<void()>& callback) override { m_OnDeinit = callback; }
	void SetOnResize(std::function<void(vec2 const)>& callback) override { m_OnResize = callback; }
	void SetOnRender(std::function<void(rhi::T_FbLoc const)>& callback) override { m_OnRender = callback; }

	void QueueDraw() override;
	bool MakeCurrent() override;

	ivec2 GetDimensions() const override;

	// Data
	///////
private:
	std::function<void(Ptr<rhi::I_RenderDevice> const)> m_OnInit;
	std::function<void()> m_OnDeinit;
	std::function<void(vec2 const)> m_OnResize;
	std::function<void(rhi::T_FbLoc const)> m_OnRender;

	Ptr<GLFWwindow> m_Window; // also serves as context

	UniquePtr<rhi::I_RenderDevice> m_RenderDevice;

	bool m_ShouldDraw = false;
};


} // namespace app
} // namespace et
