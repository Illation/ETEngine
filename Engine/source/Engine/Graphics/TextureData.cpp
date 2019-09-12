#include "stdafx.h"
#include "TextureData.h"

#include <stb/stb_image.h>
#include <stb/stb_image_resize.h>


//==============
// Texture Data
//==============


//---------------------------------
// TextureData::c-tor
//
// Create the texture and generate a new GPU texture resource
//
TextureData::TextureData(int32 const width, int32 const height, int32 const internalFormat, uint32 const format, uint32 const type, int32 const depth)
	: m_Width(width)
	, m_Height(height)
	, m_InternalFormat(internalFormat)
	, m_Format(format)
	, m_Type(type)
	, m_Depth(depth)
	, m_TargetType((depth > 1) ? E_TextureType::Texture3D : E_TextureType::Texture2D)
{
	glGenTextures(1, &m_Handle);
}

//---------------------------------
// TextureData::TextureData
//
// Create a texture of a specific type with a preexisting handle
//
TextureData::TextureData(E_TextureType const targetType, int32 const height, int32 const width)
	: m_TargetType(targetType)
	, m_Height(height)
	, m_Width(width)
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
// TextureData::GetTarget
//
// returns a textures target type
//
uint32 TextureData::GetTarget() const
{
	switch (m_TargetType)
	{
	case E_TextureType::Texture2D:
		return GL_TEXTURE_2D;

	case E_TextureType::Texture3D:
		return GL_TEXTURE_3D;

	case E_TextureType::CubeMap:
		return GL_TEXTURE_CUBE_MAP;
	}

	ET_ASSERT(true, "Unhandled texture type!");
	return GL_NONE;
}

//---------------------------------
// TextureData::Build
//
// send the data to the GPU location. Can be initialized without any image data
//
void TextureData::Build(void* data)
{
	uint32 const target = GetTarget();
	STATE->BindTexture(target, m_Handle);

	switch (m_TargetType)
	{
	case E_TextureType::Texture2D:
		glTexImage2D(target, 0, m_InternalFormat, m_Width, m_Height, 0, m_Format, m_Type, data);
		break;

	case E_TextureType::Texture3D:
		glTexImage3D(target, 0, m_InternalFormat, m_Width, m_Height, m_Depth, 0, m_Format, m_Type, data);
		break;

	case E_TextureType::CubeMap:
		ET_ASSERT(m_Width == m_Height);
		for (uint8 face = 0u; face < s_NumCubeFaces; ++face)
		{
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, 0, GL_RGB16F, m_Width, m_Height, 0, GL_RGB, GL_FLOAT, nullptr);
		}
		break;
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
	uint32 const target = GetTarget();
	STATE->BindTexture(target, m_Handle);

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
	if ((!m_Parameters.genMipMaps && params.genMipMaps) || (params.genMipMaps && force) || (params.genMipMaps && (m_MipLevels == 0u)))
	{
		glGenerateMipmap(target);
		float const largerRes = static_cast<float>(std::max(m_Width, m_Height));
		m_MipLevels = 1u + static_cast<uint8>(floor(log10(largerRes) / log10(2.f)));
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
	// check image format

	stbi_set_flip_vertically_on_load(false);
	int32 width = 0;
	int32 height = 0;
	int32 channels = 0;

	static int32 const s_TargetNumChannels = 3; // for now we only support opaque textures

	// option to load 16 bit texture
	uint8* bits = stbi_load_from_memory(data.data(), static_cast<int32>(data.size()), &width, &height, &channels, s_TargetNumChannels);

	if (bits == nullptr)
	{
		LOG("TextureAsset::LoadFromMemory > Failed to load texture bytes from data!", LogLevel::Warning);
		return false;
	}

	if ((width == 0) || (height == 0))
	{
		LOG("TextureAsset::LoadFromMemory > Image is too small to display!", LogLevel::Warning);
		stbi_image_free(bits);
		return false;
	}

	if (!etm::nearEquals(GRAPHICS.TextureScaleFactor, 1.f))
	{
		if (!m_ForceResolution)
		{
			// resize
			int32 const outWidth = static_cast<int32>(static_cast<float>(width) * GRAPHICS.TextureScaleFactor);
			int32 const outHeight = static_cast<int32>(static_cast<float>(height) * GRAPHICS.TextureScaleFactor);
			uint8* outBits = new uint8[outWidth * outHeight * channels];

			stbir_resize_uint8(bits, width, height, 0, outBits, outWidth, outHeight, 0, channels);

			stbi_image_free(bits);
			bits = outBits;
			width = outWidth;
			height = outHeight;
		}
	}

	// convert data type
	// check number of channels

	//Upload to GPU
	m_Data = new TextureData(width, height, m_UseSrgb ? GL_SRGB : GL_RGB, GL_RGB, GL_UNSIGNED_BYTE);
	m_Data->Build((void*)bits);
	m_Data->SetParameters(m_Parameters);

	stbi_image_free(bits);
	bits = nullptr;

	return true;
}
