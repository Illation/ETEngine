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
	TextureData(GLuint handle, int32 width, int32 height, int32 depth = 1);
	TextureData(int32 width, int32 height, int32 internalFormat, GLenum format, GLenum type, int32 depth = 1);
	~TextureData();

	GLuint GetHandle() { return m_Handle; }
	ivec2 GetResolution(){return ivec2(m_Width, m_Height);}

	void Build(void* data = NULL);
	void SetParameters( TextureParameters params );

	GLenum GetTarget() { return m_Depth == 1 ? GL_TEXTURE_2D : GL_TEXTURE_3D; }

	// returns true if regenerated 
	// if its a framebuffer texture upscaling won't work properly 
	// unless it is reatached to the framebuffer object
	bool Resize( ivec2 newSize );

private:
	GLuint m_Handle;

	int32 m_Width;
	int32 m_Height;

	int32 m_Depth; // a (default) value of 1 implies a 2D texture

	//Setup
	int32 m_InternalFormat = GL_RGB;
	GLenum m_Format = GL_RGB;
	GLenum m_Type = GL_FLOAT;

	TextureParameters m_Parameters;
};