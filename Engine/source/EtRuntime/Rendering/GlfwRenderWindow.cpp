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
rhi::I_GraphicsContextApi* GlfwRenderWindow::CreateContext(rhi::GraphicsContextParams const& params)
{
	m_Area.Initialize(params, m_IsHidden);
	return m_Area.GetContext();
}

//--------------------------------
// GlfwRenderWindow::SetCursorPos
//
void GlfwRenderWindow::SetCursorPos(ivec2 const pos)
{
	glfwSetCursorPos(m_Area.GetWindow(), static_cast<double>(pos.x), static_cast<double>(pos.y));
}

//---------------------------------
// GlfwRenderWindow::GetDimensions
//
ivec2 GlfwRenderWindow::GetDimensions() const
{
	ivec2 ret;
	glfwGetWindowSize(m_Area.GetWindow(), &ret.x, &ret.y);
	return ret;
}

//----------------------------
// GlfwRenderWindow::HasFocus
//
bool GlfwRenderWindow::HasFocus() const
{
	return (glfwGetWindowAttrib(m_Area.GetWindow(), GLFW_FOCUSED) != 0);
}


} // namespace rt
} // namespace et
