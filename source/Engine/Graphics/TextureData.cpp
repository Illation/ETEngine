#include "stdafx.hpp"
#include "TextureData.hpp"

TextureData::TextureData(GLuint handle, int32 width, int32 height, int32 depth):
	m_Handle(handle),
	m_Width(width),
	m_Height(height),
	m_Depth( depth )
{
}

TextureData::TextureData( int32 width, int32 height, int32 internalFormat, GLenum format, GLenum type, int32 depth ) :
	m_Width( width ),
	m_Height( height ),
	m_InternalFormat( internalFormat ),
	m_Format( format ),
	m_Type( type ), 
	m_Depth( depth )
{
	glGenTextures( 1, &m_Handle );
	ZeroMemory( &m_Parameters, sizeof( TextureParameters ) ); //ensure setting them when its called
}

TextureData::~TextureData()
{
	glDeleteTextures(1, &m_Handle);
}

void TextureData::Build( void* data /*= NULL*/ )
{
	if (m_Depth == 1)
	{
		STATE->BindTexture( GL_TEXTURE_2D, m_Handle );
		glTexImage2D(GL_TEXTURE_2D, 0, m_InternalFormat, m_Width, m_Height, 0, m_Format, m_Type, data );
	}
	else
	{
		STATE->BindTexture( GL_TEXTURE_3D, m_Handle );
		glTexImage3D(GL_TEXTURE_3D, 0, m_InternalFormat, m_Width, m_Height, m_Depth, 0, m_Format, m_Type, data );
	}
}

void TextureData::SetParameters( TextureParameters params )
{
	GLenum target = GetTarget();
	STATE->BindTexture(target, m_Handle );
	if(m_Parameters.minFilter != params.minFilter)
	{
		glTexParameteri(target, GL_TEXTURE_MIN_FILTER, params.minFilter );
	}
	if(m_Parameters.magFilter != params.magFilter)
	{
		glTexParameteri(target, GL_TEXTURE_MAG_FILTER, params.magFilter );
	}
	if(m_Parameters.wrapS != params.wrapS)
	{
		glTexParameteri(target, GL_TEXTURE_WRAP_S, params.wrapS );
	}
	if(m_Parameters.wrapT != params.wrapT)
	{
		glTexParameteri(target, GL_TEXTURE_WRAP_T, params.wrapT );
	}
	if(!etm::nearEqualsV(m_Parameters.borderColor, params.borderColor ))
	{
		glTexParameterfv(target, GL_TEXTURE_BORDER_COLOR, params.borderColor.data.data() );
	}
	if(m_Parameters.genMipMaps == false && params.genMipMaps == true)
	{
		glGenerateMipmap(target);
	}
	if(params.isDepthTex && m_Parameters.compareMode != params.compareMode)
	{
		glTexParameteri(target, GL_TEXTURE_COMPARE_MODE, params.compareMode );//shadow map comp mode
	}
	if (m_Depth > 1)
	{
		if (m_Parameters.wrapR != params.wrapR)
		{
			glTexParameteri(target, GL_TEXTURE_WRAP_R, params.wrapR);
		}
	}
	m_Parameters = params;
}

bool TextureData::Resize( ivec2 newSize )
{
	if(newSize.x > m_Width || newSize.y > m_Height)
	{
		m_Width = newSize.x; m_Height = newSize.y;
		glDeleteTextures(1, &m_Handle);
		glGenTextures( 1, &m_Handle );
		Build();
		auto tempParams = m_Parameters;
		ZeroMemory( &m_Parameters, sizeof( TextureParameters ) ); //ensure setting them when its called
		SetParameters( tempParams );
		return true;
	}
	m_Width = newSize.x; m_Height = newSize.y;
	Build();
	return false;
}


GLuint HDRMap::GetBrdfLutHandle()
{
	return m_BrdfLutHandle;
}