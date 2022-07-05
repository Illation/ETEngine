#pragma once

#include <EtRendering/GraphicsContext/RenderWindow.h>

#include "GlfwRenderArea.h"


namespace et {
namespace rt {


//---------------------------------
// GlfwRenderWindow
//
// Render window using GLFW windows as a subsystem, managing an openGL context, and containing a single window sized render area
//
class GlfwRenderWindow final : public render::RenderWindow
{
	// construct destruct
	//-------------------
public:
	GlfwRenderWindow(bool const hidden = false) : render::RenderWindow(), m_IsHidden(hidden) {}
	~GlfwRenderWindow() = default;

	// accessors
	//-----------
	GlfwRenderArea& GetArea() { return m_Area; }

	// Render Window Interface
	//-------------------------
protected:
	render::I_GraphicsContextApi* CreateContext(render::GraphicsContextParams const& params) override;
	void SetCursorPos(ivec2 const pos) override;

	ivec2 GetDimensions() const override;
	bool HasFocus() const override;

	// Data
	///////
private:

	bool m_IsHidden;
	GlfwRenderArea m_Area;
};


} // namespace rt
} // namespace et

