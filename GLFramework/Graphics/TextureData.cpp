#include "stdafx.hpp"
#include "TextureData.hpp"

TextureData::TextureData(GLuint handle, int width, int height):
	m_Handle(handle),
	m_Width(width),
	m_Height(height)
{
	
}


TextureData::~TextureData()
{
	glDeleteTextures(1, &m_Handle);
}
