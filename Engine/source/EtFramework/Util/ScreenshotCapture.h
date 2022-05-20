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
	void Take(render::Viewport* const viewport);

	// utility
	//----------
private:
	void OnViewportPostFlush(render::T_FbLoc const targetFb);
	std::string GetFileName();

	// Data
	///////

	core::Directory* m_BaseDir = nullptr;
	render::Viewport* m_Viewport = nullptr;
	render::T_ViewportEventCallbackId m_VPCallbackId = render::T_ViewportEventDispatcher::INVALID_ID;
};


} // namespace fw
} // namespace et
