#pragma once


// forward
class Directory;


//---------------------------------
// ScreenshotCapture
//
// Captures the content of a viewport and outputs it to an image file
//
class ScreenshotCapture final : public I_ViewportListener
{
	// definitions
	//---------------
private:
	friend class RenderingSystems;

	// construct destruct
	//---------------------
	ScreenshotCapture() = default;
	virtual ~ScreenshotCapture();

	void Initialize(std::string basePath);

	// functionality
	//----------------
public:
	void Take(Viewport* const viewport);

	// Viewport Listener interface
	//-----------------------------
protected:

	void OnViewportPreRender(T_FbLoc const) override {}
	void OnViewportPostFlush(T_FbLoc const targetFb) override;

	// utility
	//----------
private:
	std::string GetFileName();

	// Data
	///////

	Directory* m_BaseDir = nullptr;
	Viewport* m_Viewport = nullptr;
};