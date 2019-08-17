#include "stdafx.h"
#include "TextureData.h"


//==============
// Texture Data
//==============


//---------------------------------
// TextureData::c-tor
//
// Create the texture object from an existing GPU resource
//
TextureData::TextureData(GLuint handle, int32 width, int32 height, int32 depth):
	m_Handle(handle),
	m_Width(width),
	m_Height(height),
	m_Depth( depth )
{ }

//---------------------------------
// TextureData::c-tor
//
// Create the texture and generate a new GPU texture resource
//
TextureData::TextureData( int32 width, int32 height, int32 internalFormat, GLenum format, GLenum type, int32 depth ) :
	m_Width( width ),
	m_Height( height ),
	m_InternalFormat( internalFormat ),
	m_Format( format ),
	m_Type( type ), 
	m_Depth( depth )
{
	glGenTextures(1, &m_Handle);
}

//---------------------------------
// TextureData::d-tor
//
// Destroys the GPU resource
//
TextureData::~TextureData()
{
	glDeleteTextures(1, &m_Handle);
}

//---------------------------------
// TextureData::Build
//
// send the data to the GPU location. Can be initialized without any image data
//
void TextureData::Build(void* data)
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

//---------------------------------
// TextureData::SetParameters
//
// Sets parameters on a texture, can optionally be enforced
//
void TextureData::SetParameters(TextureParameters const& params, bool const force)
{
	GLenum target = GetTarget();
	STATE->BindTexture(target, m_Handle );

	if((m_Parameters.minFilter != params.minFilter) || 
		(m_Parameters.mipFilter != params.mipFilter) || 
		(m_Parameters.genMipMaps != params.genMipMaps) ||
		force)
	{
		int32 minFilter = GetMinFilter(params.minFilter, params.mipFilter, params.genMipMaps);
		ET_ASSERT(minFilter != 0);

		glTexParameteri(target, GL_TEXTURE_MIN_FILTER, minFilter);
	}

	if ((m_Parameters.magFilter != params.magFilter) || force)
	{
		int32 filter = GetFilter(params.magFilter);
		ET_ASSERT(filter != 0);

		glTexParameteri(target, GL_TEXTURE_MAG_FILTER, filter);
	}

	if ((m_Parameters.wrapS != params.wrapS) || force)
	{
		glTexParameteri(target, GL_TEXTURE_WRAP_S, GetWrapMode(params.wrapS));
	}

	if ((m_Parameters.wrapT != params.wrapT) || force)
	{
		glTexParameteri(target, GL_TEXTURE_WRAP_T, GetWrapMode(params.wrapT));
	}

	if ((m_Depth > 1) && ((m_Parameters.wrapR != params.wrapR) || force))
	{
		glTexParameteri(target, GL_TEXTURE_WRAP_R, GetWrapMode(params.wrapR));
	}

	if (!etm::nearEqualsV(m_Parameters.borderColor, params.borderColor ) || force)
	{
		glTexParameterfv(target, GL_TEXTURE_BORDER_COLOR, params.borderColor.data.data());
	}

	if ((!m_Parameters.genMipMaps && params.genMipMaps) || (params.genMipMaps && force) || (params.genMipMaps && !m_HasMipData))
	{
		glGenerateMipmap(target);
		m_HasMipData = true;
	}

	if ((params.isDepthTex && (m_Parameters.compareMode != params.compareMode)) || (params.isDepthTex && force))
	{
		glTexParameteri(target, GL_TEXTURE_COMPARE_MODE, GetCompareMode(params.compareMode));//shadow map comp mode
	}

	m_Parameters = params;
}

//---------------------------------
// TextureData::Resize
//
// returns true if regenerated 
//  - if the texture is a framebuffer texture upscaling won't work properly unless it is reatached to the framebuffer object
//
bool TextureData::Resize(ivec2 const& newSize)
{
	m_Width = newSize.x; 
	m_Height = newSize.y;

	bool const regenerate = (newSize.x > m_Width) || (newSize.y > m_Height);
	if (regenerate)
	{
		glDeleteTextures(1, &m_Handle);
		glGenTextures(1, &m_Handle);
	}

	Build();

	if (regenerate)
	{
		SetParameters(m_Parameters, true);
	}

	return regenerate;
}
