#pragma once

#include <EtRendering/GraphicsContext/GraphicsContext.h>

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
	GlfwRenderWindow() : render::RenderWindow() {}
	~GlfwRenderWindow() = default;

	// accessors
	//-----------
	GlfwRenderArea& GetArea() { return m_Area; }

	// Render Window Interface
	//-------------------------
protected:
	render::I_GraphicsContextApi* CreateContext(render::GraphicsContextParams const& params) override;

	// Data
	///////
private:

	GlfwRenderArea m_Area;
};


} // namespace rt
} // namespace et

