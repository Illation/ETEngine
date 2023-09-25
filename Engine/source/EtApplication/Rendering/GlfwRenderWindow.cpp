#include "stdafx.h"
#include "GlfwRenderWindow.h"

#ifdef ET_PLATFORM_WIN
#	define GLFW_EXPOSE_NATIVE_WIN32
#endif
#include <GLFW/glfw3native.h>

#include <lunasvg.h>

#include <EtCore/Content/ResourceManager.h>

#include <EtGUI/Context/GuiDocument.h>


namespace et {
namespace app {


//=====================
// GLFW Render Window
//=====================


//---------------------------------
// GlfwRenderWindow::SetDimensions
//
void GlfwRenderWindow::SetDimensions(ivec2 const dim)
{
	glfwSetWindowSize(m_Area.GetWindow(), dim.x, dim.y);
}

//-----------------------------
// GlfwRenderWindow::SetIcon
//
// Expects an SVG image in a gui document asset
//
bool GlfwRenderWindow::SetIcon(core::HashString const svgAssetId)
{
	if (svgAssetId.IsEmpty())
	{
		glfwSetWindowIcon(m_Area.GetWindow(), 0, nullptr);
		return true;
	}

	AssetPtr<gui::GuiDocument> const svgAsset = core::ResourceManager::Instance()->GetAssetData<gui::GuiDocument>(svgAssetId);
	if (svgAsset == nullptr)
	{
		ET_WARNING("failed to load gui document '%s'", svgAssetId.ToStringDbg());
		return false;
	}

	std::unique_ptr<lunasvg::Document> const doc = lunasvg::Document::loadFromData(svgAsset->GetText(), svgAsset->GetLength());
	if (doc == nullptr)
	{
		ET_WARNING("failed to load SVG document from asset '%s'", svgAssetId.ToStringDbg());
		return false;
	}

	static uint32 const sizes[] = { /*16, 24, */32, /*48,*/ 256 };
	std::vector<GLFWimage> images;
	std::vector<lunasvg::Bitmap> bitmaps;
	for (uint32 const size : sizes)
	{
		images.emplace_back();
		GLFWimage& image = images.back();

		bitmaps.emplace_back(doc->renderToBitmap(size, size));
		lunasvg::Bitmap& bitmap = bitmaps.back();
		bitmap.convertToRGBA();

		image.width = static_cast<int>(bitmap.width());
		image.height = static_cast<int>(bitmap.height());
		image.pixels = reinterpret_cast<unsigned char*>(bitmap.data());
	}

	glfwSetWindowIcon(m_Area.GetWindow(), static_cast<int>(images.size()), images.data());
	return true;
}

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


} // namespace app
} // namespace et
