#pragma once

#include <EtRHI/GraphicsContext/RenderWindow.h>

#include "GlfwRenderArea.h"


namespace et {
namespace app {


//---------------------------------
// GlfwRenderWindow
//
// Render window using GLFW windows as a subsystem, managing an openGL context, and containing a single window sized render area
//
class GlfwRenderWindow final : public rhi::RenderWindow
{
	// construct destruct
	//-------------------
public:
	GlfwRenderWindow(core::WindowSettings const& windowSettings, bool const hidden = false) 
		: rhi::RenderWindow(), m_WindowSettings(windowSettings), m_IsHidden(hidden) {}
	~GlfwRenderWindow() = default;

	// functionality
	//---------------
	void StartDrag();

	// accessors
	//-----------
	GlfwRenderArea& GetArea() { return m_Area; }

	// Render Window Interface
	//-------------------------
protected:
	Ptr<rhi::I_RenderDevice> CreateRenderDevice(rhi::RenderDeviceParams const& params) override;
	void SetCursorPos(ivec2 const pos) override;

	ivec2 GetDimensions() const override;
	bool HasFocus() const override;

	// Data
	///////
private:

	GlfwRenderArea m_Area;

	core::WindowSettings m_WindowSettings;
	bool m_IsHidden;
};


} // namespace app
} // namespace et

