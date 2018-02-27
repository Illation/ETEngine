#pragma once
#include "Singleton.hpp"
#include <FreeImage.h>

class Directory;

class ScreenshotCapture : public Singleton<ScreenshotCapture>
{
public:
	void Take() { m_Take = true; }

	void SetFormat(FREE_IMAGE_FORMAT val) { m_Format = val; }

private:
	friend class Singleton<ScreenshotCapture>;
	friend class RenderPipeline;
	friend class AbstractFramework;

	ScreenshotCapture() = default;
	virtual ~ScreenshotCapture();

	void Initialize(std::string m_BasePath);

	std::string GetFileName();
	void HandleCapture();//To be called by Render pipeline

	FREE_IMAGE_FORMAT m_Format = FIF_JPEG;

	Directory* m_BaseDir = nullptr;
	bool m_Take = false;
};