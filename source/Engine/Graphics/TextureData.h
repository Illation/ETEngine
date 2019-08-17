#pragma once
#include <string>
#include <vector>

#include "TextureParameters.h"


//---------------------------------
// TextureData
//
// Handle to a texture object on the GPU
//
class TextureData
{
public:
	// c-tor d-tor
	//------------
	TextureData(GLuint handle, int32 width, int32 height, int32 depth = 1);
	TextureData(int32 width, int32 height, int32 internalFormat, GLenum format, GLenum type, int32 depth = 1);
	~TextureData();

	// Accessors
	//----------
	GLuint GetHandle() const { return m_Handle; }
	ivec2 GetResolution() const { return ivec2(m_Width, m_Height); }
	GLenum GetTarget() const { return m_Depth == 1 ? GL_TEXTURE_2D : GL_TEXTURE_3D; }

	// Functionality
	//--------------
	void Build(void* data = nullptr);
	void SetParameters(TextureParameters const& params, bool const force = false);
	bool Resize(ivec2 const& newSize);

private:
	// Data
	///////

	// GPU data
	GLuint m_Handle;
	bool m_HasMipData = false;

	// Resolution
	int32 m_Width;
	int32 m_Height;
	int32 m_Depth = 1; // a (default) value of 1 implies a 2D texture

	// Format
	int32 m_InternalFormat = GL_RGB;
	GLenum m_Format = GL_RGB;
	GLenum m_Type = GL_FLOAT;

	TextureParameters m_Parameters;
};