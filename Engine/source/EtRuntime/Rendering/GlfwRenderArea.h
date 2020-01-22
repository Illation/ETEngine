#pragma once

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <EtRendering/GraphicsContext/RenderArea.h>


namespace et {
namespace rt {


//---------------------------------
// GlfwRenderArea
//
// Render area using GLFW windows as a subsystem, managing an openGL context
//
class GlfwRenderArea final : public render::I_RenderArea
{
	// construct destruct
	//-------------------
public:
	GlfwRenderArea() : render::I_RenderArea() {}
	virtual ~GlfwRenderArea() = default;

	void Initialize();
	void Uninitialize();

	// functionality
	//---------------
	void Update();
	void SetVSyncEnabled(bool const val);

	void SetSize(ivec2 const size);

	// accessors
	//-----------
	GLFWwindow* GetWindow() const { return m_Window; }

	// Render Area Interface
	//-----------------------
protected:
	void SetOnInit(std::function<void(render::I_GraphicsApiContext* const)>& callback) override { m_OnInit = callback; }
	void SetOnDeinit(std::function<void()>& callback) override { m_OnDeinit = callback; }
	void SetOnResize(std::function<void(vec2 const)>& callback) override { m_OnResize = callback; }
	void SetOnRender(std::function<void(render::T_FbLoc const)>& callback) override { m_OnRender = callback; }

	void QueueDraw() override;
	bool MakeCurrent() override;

	ivec2 GetDimensions() const override;

	// Data
	///////
private:
	std::function<void(render::I_GraphicsApiContext* const)> m_OnInit;
	std::function<void()> m_OnDeinit;
	std::function<void(vec2 const)> m_OnResize;
	std::function<void(render::T_FbLoc const)> m_OnRender;

	GLFWwindow* m_Window = nullptr; // also serves as context

	render::I_GraphicsApiContext* m_Context = nullptr;

	bool m_ShouldDraw = false;
};


} // namespace rt
} // namespace et
