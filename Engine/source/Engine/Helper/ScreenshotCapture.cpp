#include "stdafx.h"
#include "ScreenshotCapture.h"

#include <cctype>
#include <locale>

#include <stb/stb_image_write.h>

#include <EtCore/FileSystem/Entry.h>


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

	std::string extension("jpg");

	auto entries = m_BaseDir->GetChildrenByExt(extension);

	std::string baseName = WINDOW.Title;
	for (uint32 i = 0; i < (uint32)baseName.size(); ++i)
	{
		if (std::isspace(baseName[i]))baseName[i] = '_';
	}
	uint32 number = 0;

	auto constructFileName = [&]() 
	{ 
		return baseName + "_" + std::to_string(number) + "." + extension; 
	};

	while (!(std::find_if(entries.begin(), entries.end(), [&](Entry* entry) 
		{ 
			return entry->GetNameOnly() == constructFileName(); 
		} ) == entries.end()))
	{
		number++;
	}

	std::string basePath = m_BaseDir->GetPath();
	m_BaseDir->Unmount();

	return basePath + constructFileName();
}

void ScreenshotCapture::HandleCapture()
{
	if (!m_Take)
	{
		return;
	}

	std::string filename = GetFileName();

	// Make the BYTE array, factor of 3 because it's RBG.
	uint8* pixels = new uint8[3 * WINDOW.Width * WINDOW.Height];

	glReadPixels(0, 0, WINDOW.Width, WINDOW.Height, GL_RGB, GL_UNSIGNED_BYTE, pixels);

	stbi_flip_vertically_on_write(true);
	if (stbi_write_jpg(filename.c_str(), WINDOW.Width, WINDOW.Height, 3, pixels, 90) != 0)
	{	
		LOG("Screenshot saved to: " + filename);
	}
	else
	{
		LOG("Failed to save Screenshot to: " + filename, Warning);
	}

	// Free resources
	delete[] pixels;

	m_Take = false;
}
