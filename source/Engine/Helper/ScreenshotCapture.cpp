#include "stdafx.hpp"
#include "ScreenshotCapture.h"

#include <functional>
#include <cctype>
#include <locale>

#include "FileSystem/Entry.h"

ScreenshotCapture::~ScreenshotCapture()
{
	delete m_BaseDir;
}

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
	m_BaseDir = new Directory(basePath, nullptr, true);
}

std::string ScreenshotCapture::GetFileName()
{
	if (!m_BaseDir->Mount(true))
	{
		LOG("Failed to mount screenshot directory", Warning);
		return "";
	}

	std::string extensions = FreeImage_GetFIFExtensionList(m_Format);
	std::string extension = extensions.substr(0, extensions.find(','));

	auto entries = m_BaseDir->GetChildrenByExt(extension);

	std::string baseName = WINDOW.Title;
	for (uint32 i = 0; i < (uint32)baseName.size(); ++i)
	{
		if (std::isspace(baseName[i]))baseName[i] = '_';
	}
	uint32 number = 0;

	std::function<std::string()> constructFileName = [&]() { return baseName + "_" + std::to_string(number) + "." + extension; };

	while (!(std::find_if(entries.begin(), entries.end(), [&](Entry* entry) { return entry->GetName() == constructFileName(); } ) == entries.end()))
	{
		number++;
	}

	std::string basePath = m_BaseDir->GetPath() + m_BaseDir->GetName();
	m_BaseDir->Unmount();

	return basePath + constructFileName();
}

void ScreenshotCapture::HandleCapture()
{
	if (!m_Take)return;

	std::string filename = GetFileName();

	// Make the BYTE array, factor of 3 because it's RBG.
	uint8* pixels = new uint8[3 * WINDOW.Width * WINDOW.Height];

	glReadPixels(0, 0, WINDOW.Width, WINDOW.Height, GL_BGR, GL_UNSIGNED_BYTE, pixels);

	// Convert to FreeImage format & save to file
	FIBITMAP* image = FreeImage_ConvertFromRawBits(pixels, WINDOW.Width, WINDOW.Height, 3 * WINDOW.Width, 24, 0xFF0000, 0x00FF00, 0x0000FF, false);
	if (FreeImage_Save(m_Format, image, filename.c_str(), JPEG_QUALITYSUPERB))
	{
		LOG("Screenshot saved to: " + filename);
	}
	else
	{
		LOG("Failed to save Screenshot to: " + filename, Warning);
	}

	// Free resources
	FreeImage_Unload(image);
	delete[] pixels;

	m_Take = false;
}
