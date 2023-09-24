#include "stdafx.h"
#include "GlfwRenderWindow.h"

#ifdef ET_PLATFORM_WIN
#	define GLFW_EXPOSE_NATIVE_WIN32
#endif
#include <GLFW/glfw3native.h>


namespace et {
namespace app {


//=====================
// GLFW Render Window
//=====================


//---------------------------------
// GlfwRenderWindow::CreateRenderDevice
//
Ptr<rhi::I_RenderDevice> GlfwRenderWindow::CreateRenderDevice(rhi::RenderDeviceParams const& params)
{
	m_Area.Initialize(params, m_IsHidden, m_WindowSettings);
	return ToPtr(m_Area.GetRenderDevice());
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

//-----------------------------
// GlfwRenderWindow::StartDrag
//
void GlfwRenderWindow::SetDimensions(ivec2 const dim)
{
	glfwSetWindowSize(m_Area.GetWindow(), dim.x, dim.y);
}


} // namespace app
} // namespace et
