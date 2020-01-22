#pragma once


// forward
class Directory;


namespace et {
namespace fw {


//---------------------------------
// ScreenshotCapture
//
// Captures the content of a viewport and outputs it to an image file
//
class ScreenshotCapture final : public render::I_ViewportListener
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

	// Viewport Listener interface
	//-----------------------------
protected:

	void OnViewportPreRender(render::T_FbLoc const) override {}
	void OnViewportPostFlush(render::T_FbLoc const targetFb) override;

	// utility
	//----------
private:
	std::string GetFileName();

	// Data
	///////

	Directory* m_BaseDir = nullptr;
	render::Viewport* m_Viewport = nullptr;
};


} // namespace fw
} // namespace et
