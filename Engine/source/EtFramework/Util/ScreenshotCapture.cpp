#include "stdafx.h"
#include "ScreenshotCapture.h"

#include <cctype>
#include <locale>

#include <stb/stb_image_write.h>

#include <EtCore/FileSystem/Entry.h>


namespace et {
namespace fw {


//======================
// Screenshot Capture
//======================


//---------------------------------
// ScreenshotCapture::d-tor
//
ScreenshotCapture::~ScreenshotCapture()
{
	delete m_BaseDir;
}

//---------------------------------
// ScreenshotCapture::Initialize
//
// Mount all screenshot files that are in the desired directory
//
void ScreenshotCapture::Initialize(std::string basePath)
{
	if (basePath.size() == 0)
	{
		basePath = "./";
	}
	else if (basePath[basePath.size()-1] != '/')
	{
		basePath += '/';
	}
	m_BaseDir = new core::Directory(basePath, nullptr, true);
}

//---------------------------------
// ScreenshotCapture::Take
//
// Reigster ourselves with the requested viewport. the actual capture is deferred until rendering is completed
//
void ScreenshotCapture::Take(render::Viewport* const viewport)
{
	if (m_Viewport == nullptr)
	{
		m_Viewport = viewport;

		m_VPCallbackId = m_Viewport->GetEventDispatcher().Register(render::E_ViewportEvent::VP_PostFlush,
			render::T_ViewportEventCallback([this](render::T_ViewportEventFlags const, render::ViewportEventData const* const data) -> void
				{
					OnViewportPostFlush(data->targetFb);
				}));
	}
	else
	{
		ET_TRACE_W(ET_CTX_FRAMEWORK, "ScreenshotCapture::Take > some system is already taking a capture of a viewport!");
	}
}

//---------------------------------
// ScreenshotCapture::OnViewportPostFlush
//
// We wait for the viewports renderer to finish and the API to flush so the performance impact is minimal, 
//  - then read the pixels from the GPU into a file and stop listening for viewport events again until the next capture request
//
void ScreenshotCapture::OnViewportPostFlush(render::T_FbLoc const targetFb)
{
	using namespace render;

	ET_UNUSED(targetFb); // this should already be the current framebuffer

	std::string filename = GetFileName();

	ivec2 const dim = m_Viewport->GetDimensions();

	// Make the BYTE array, factor of 3 because it's RBG.
	uint8* pixels = new uint8[3 * dim.x * dim.y];

	I_GraphicsContextApi* const api = m_Viewport->GetApiContext();
	api->Finish();
	api->ReadPixels(ivec2(0), dim, E_ColorFormat::RGB, E_DataType::UByte, pixels);

	stbi_flip_vertically_on_write(true);
	if (stbi_write_jpg(filename.c_str(), dim.x, dim.y, 3, pixels, 90) != 0)
	{	
		ET_LOG_I(ET_CTX_FRAMEWORK, "Screenshot saved to: %s", filename.c_str());
	}
	else
	{
		ET_LOG_W(ET_CTX_FRAMEWORK, "Failed to save Screenshot to: %s", filename.c_str());
	}

	// Free resources
	delete[] pixels;

	m_Viewport->GetEventDispatcher().Unregister(m_VPCallbackId);
	m_Viewport = nullptr;
}

//---------------------------------
// ScreenshotCapture::GetFileName
//
// Find a filename in the screenshot directory that hasn't already been used
//
std::string ScreenshotCapture::GetFileName()
{
	if (!m_BaseDir->Mount(true))
	{
		ET_TRACE_W(ET_CTX_FRAMEWORK, "Failed to mount screenshot directory");
		return "";
	}

	std::string extension("jpg");

	auto entries = m_BaseDir->GetChildrenByExt(extension);

	std::string baseName = Config::GetInstance()->GetWindow().Title;
	for (uint32 i = 0; i < (uint32)baseName.size(); ++i)
	{
		if (std::isspace(baseName[i]))baseName[i] = '_';
	}
	uint32 number = 0;

	auto constructFileName = [&]()
	{
		return baseName + "_" + std::to_string(number) + "." + extension;
	};

	while (!(std::find_if(entries.begin(), entries.end(), [&](core::Entry* entry)
	{
		return entry->GetNameOnly() == constructFileName();
	}) == entries.end()))
	{
		number++;
	}

	std::string basePath = m_BaseDir->GetPath();
	m_BaseDir->Unmount();

	return basePath + constructFileName();
}


} // namespace fw
} // namespace et
