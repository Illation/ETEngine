#include "stdafx.h"
#include "TextureData.h"


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

	if((m_Parameters.minFilter != params.minFilter) || 
		(m_Parameters.mipFilter != params.mipFilter) || 
		(m_Parameters.genMipMaps != params.genMipMaps))
	{
		int32 minFilter = GetMinFilter(params.minFilter, params.mipFilter, params.genMipMaps);
		ET_ASSERT(minFilter != 0);

		glTexParameteri(target, GL_TEXTURE_MIN_FILTER, minFilter);
	}

	if(m_Parameters.magFilter != params.magFilter)
	{
		int32 filter = GetFilter(params.magFilter);
		ET_ASSERT(filter != 0);

		glTexParameteri(target, GL_TEXTURE_MAG_FILTER, filter);
	}

	if(m_Parameters.wrapS != params.wrapS)
	{
		glTexParameteri(target, GL_TEXTURE_WRAP_S, GetWrapMode(params.wrapS));
	}
	if(m_Parameters.wrapT != params.wrapT)
	{
		glTexParameteri(target, GL_TEXTURE_WRAP_T, GetWrapMode(params.wrapT));
	}
	if (m_Depth > 1 && m_Parameters.wrapR != params.wrapR)
	{
		glTexParameteri(target, GL_TEXTURE_WRAP_R, GetWrapMode(params.wrapR));
	}

	if(!etm::nearEqualsV(m_Parameters.borderColor, params.borderColor ))
	{
		glTexParameterfv(target, GL_TEXTURE_BORDER_COLOR, params.borderColor.data.data());
	}
	if(m_Parameters.genMipMaps == false && params.genMipMaps == true)
	{
		glGenerateMipmap(target);
	}
	if(params.isDepthTex && m_Parameters.compareMode != params.compareMode)
	{
		glTexParameteri(target, GL_TEXTURE_COMPARE_MODE, GetCompareMode(params.compareMode));//shadow map comp mode
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
