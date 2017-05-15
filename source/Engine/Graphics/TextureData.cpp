#include "stdafx.hpp"
#include "TextureData.hpp"

TextureData::TextureData(GLuint handle, int32 width, int32 height):
	m_Handle(handle),
	m_Width(width),
	m_Height(height)
{
	
}


TextureData::~TextureData()
{
	glDeleteTextures(1, &m_Handle);
}

GLuint HDRMap::GetBrdfLutHandle()
{
	return m_BrdfLutHandle;
}