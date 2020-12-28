#include "stdafx.h"
#include "GlfwRenderWindow.h"


namespace et {
namespace rt {


//=====================
// GLFW Render Window
//=====================


//---------------------------------
// GlfwRenderWindow::CreateContext
//
render::I_GraphicsContextApi* GlfwRenderWindow::CreateContext(render::GraphicsContextParams const& params)
{
	m_Area.Initialize(params, m_IsHidden);
	return m_Area.GetContext();
}


} // namespace rt
} // namespace et
