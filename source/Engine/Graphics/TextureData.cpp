#include "stdafx.h"
#include "TextureData.h"

#include <FreeImage.h>


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
// This function should be called either way when a new texture is created, 
//  - because if we leave the default parameters OpenGL expects us to generate a mip map, or we can disable mip maps which will change the min filter
//
void TextureData::SetParameters(TextureParameters const& params, bool const force)
{
	GLenum target = GetTarget();
	STATE->BindTexture(target, m_Handle );

	// filter options
	//---------------
	// in the future it may make sense to create filter groups so that things such as anisotropy can be set globally
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

	// address mode
	//-------------
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

	// border color
	if (!etm::nearEqualsV(m_Parameters.borderColor, params.borderColor ) || force)
	{
		glTexParameterfv(target, GL_TEXTURE_BORDER_COLOR, params.borderColor.data.data());
	}

	// other
	//-------
	if ((params.isDepthTex && (m_Parameters.compareMode != params.compareMode)) || (params.isDepthTex && force))
	{
		glTexParameteri(target, GL_TEXTURE_COMPARE_MODE, GetCompareMode(params.compareMode));//shadow map comp mode
	}

	// generate mip maps if we must
	//-----------------------------
	if ((!m_Parameters.genMipMaps && params.genMipMaps) || (params.genMipMaps && force) || (params.genMipMaps && !m_HasMipData))
	{
		glGenerateMipmap(target);
		m_HasMipData = true;
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


//===================
// Texture Asset
//===================


// reflection
RTTR_REGISTRATION
{
	using namespace rttr;

	registration::class_<TextureAsset>("texture asset")
		.constructor<TextureAsset const&>()
		.constructor<>()(rttr::detail::as_object())
		.property("use SRGB", &TextureAsset::m_UseSrgb)
		.property("force resolution", &TextureAsset::m_ForceResolution)
		.property("parameters", &TextureAsset::m_Parameters);

	rttr::type::register_converter_func([](TextureAsset& asset, bool& ok) -> I_Asset*
	{
		ok = true;
		return new TextureAsset(asset);
	});
}
DEFINE_FORCED_LINKING(TextureAsset) // force the shader class to be linked as it is only used in reflection


//---------------------------------
// TextureAsset::LoadFromMemory
//
// Load texture data from binary asset content, and place it on the GPU
//
bool TextureAsset::LoadFromMemory(std::vector<uint8> const& data)
{
	FREE_IMAGE_FORMAT fif = FIF_UNKNOWN;
	fif = FreeImage_GetFileType(GetName().c_str(), 0);
	if (fif == FIF_UNKNOWN)
	{
		fif = FreeImage_GetFIFFromFilename(GetName().c_str());
	}

	if (fif == FIF_UNKNOWN)
	{
		LOG("TextureAsset::LoadFromMemory > Cannot extract texture format from asset name!", LogLevel::Warning);
		return false;
	}

	FIBITMAP* pImage = nullptr;
	if (FreeImage_FIFSupportsReading(fif))
	{
		// Create a FreeImage datastream from load data. 
		// Since FreeImage requires this to be non const, we need to copy the entire load data into a temporary non const array
		BYTE* tempData = (BYTE*)malloc(data.size());
		std::copy(data.begin(), data.end(), tempData);
		FIMEMORY* stream = FreeImage_OpenMemory(tempData, static_cast<DWORD>(data.size()));
		ET_ASSERT(stream != nullptr);

		pImage = FreeImage_LoadFromMemory(fif, stream);
		FreeImage_CloseMemory(stream);
		free(tempData);
	}

	if (pImage == nullptr)
	{
		LOG("TextureAsset::LoadFromMemory > Failed to load FIBITMAP from data!", LogLevel::Warning);
		return false;
	}

	//Get dimensions and downscale if necessary
	uint32 width = FreeImage_GetWidth(pImage);
	uint32 height = FreeImage_GetHeight(pImage);
	if (!etm::nearEquals(GRAPHICS.TextureScaleFactor, 1.f))
	{
		if (!m_ForceResolution)
		{
			FIBITMAP* oldImage = pImage;
			pImage = FreeImage_Rescale(pImage, (int32)(width*GRAPHICS.TextureScaleFactor), (int32)(height*GRAPHICS.TextureScaleFactor));
			FreeImage_Unload(oldImage);
			width = FreeImage_GetWidth(pImage);
			height = FreeImage_GetHeight(pImage);
		}
	}

	//Convert into opengl compatible format
	FreeImage_FlipVertical(pImage);
	FIBITMAP* oldImage = pImage;
	pImage = FreeImage_ConvertToType(pImage, FIT_RGBF);
	FreeImage_Unload(oldImage);

	//Get and validate data pointer
	uint8* bits = FreeImage_GetBits(pImage);
	if ((bits == 0) || (width == 0) || (height == 0))
	{
		LOG("TextureAsset::LoadFromMemory > Failed to get image bits from FIBITMAP!", LogLevel::Warning);
		FreeImage_Unload(pImage);
		return false;
	}

	//Upload to GPU
	m_Data = new TextureData(width, height, m_UseSrgb ? GL_SRGB : GL_RGB, GL_RGB, GL_FLOAT);
	m_Data->Build((void*)bits);
	m_Data->SetParameters(m_Parameters);

	FreeImage_Unload(pImage);//Destroy CPU side data

	return true;
}
