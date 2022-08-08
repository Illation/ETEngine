#pragma once


// forward
namespace et { namespace core {
	class Directory;
} }


namespace et {
namespace fw {


//---------------------------------
// ScreenshotCapture
//
// Captures the content of a viewport and outputs it to an image file
//
class ScreenshotCapture final
{
	// construct destruct
	//---------------------
public:
	ScreenshotCapture() = default;
	virtual ~ScreenshotCapture();

	void Initialize(std::string basePath);

	// functionality
	//----------------
	void Take(rhi::Viewport* const viewport);

	// utility
	//----------
private:
	void OnViewportPostFlush(rhi::T_FbLoc const targetFb);
	std::string GetFileName();

	// Data
	///////

	core::Directory* m_BaseDir = nullptr;
	rhi::Viewport* m_Viewport = nullptr;
	rhi::T_ViewportEventCallbackId m_VPCallbackId = rhi::T_ViewportEventDispatcher::INVALID_ID;
};


} // namespace fw
} // namespace et
