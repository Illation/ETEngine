#include "stdafx.h"
#include "TextureFormat.h"
#include <EtCore/Reflection/ReflectionUtil.h>


namespace et {
namespace render {


// reflection
RTTR_REGISTRATION
{
	using namespace rttr;

	registration::enumeration<TextureFormat::E_Srgb>("E_SrgbSetting") (
		value("None", TextureFormat::E_Srgb::None),
		value("OnLoad", TextureFormat::E_Srgb::OnLoad));
}

DEFINE_FORCED_LINKING(TextureFormat) // force the linker to include this unit

// static
std::string const TextureFormat::s_TextureFileExt("ettex");
std::string const TextureFormat::s_Header("ETTEX");
size_t const TextureFormat::s_BlockPixelCount = 16u;


//---------------------------------
// TextureFormat::IsCompressedFormat
//
bool TextureFormat::IsCompressedFormat(E_ColorFormat const format)
{
	switch (format)
	{
	case E_ColorFormat::BC1_RGB:
	case E_ColorFormat::BC1_RGBA:
	case E_ColorFormat::BC1_SRGB:
	case E_ColorFormat::BC1_SRGBA:

	case E_ColorFormat::BC3_RGBA:
	case E_ColorFormat::BC3_SRGBA:

	case E_ColorFormat::BC4_Red:
	case E_ColorFormat::BC4_Red_Signed:

	case E_ColorFormat::BC5_RG:
	case E_ColorFormat::BC5_RG_Signed:

	case E_ColorFormat::BC6H_RGB:
	case E_ColorFormat::BC6H_RGB_Signed:

	case E_ColorFormat::BC7_RGBA:
	case E_ColorFormat::BC7_SRGBA:
		return true;
	}

	return false;
}

//----------------------------------
// TextureFormat::GetBlockByteCount
//
uint8 TextureFormat::GetBlockByteCount(E_ColorFormat const format)
{
	switch (format)
	{
	case E_ColorFormat::BC1_RGB:
	case E_ColorFormat::BC1_RGBA:
	case E_ColorFormat::BC1_SRGB:
	case E_ColorFormat::BC1_SRGBA:

	case E_ColorFormat::BC4_Red:
	case E_ColorFormat::BC4_Red_Signed:
		return 8u;

	case E_ColorFormat::BC3_RGBA:
	case E_ColorFormat::BC3_SRGBA:

	case E_ColorFormat::BC5_RG:
	case E_ColorFormat::BC5_RG_Signed:

	case E_ColorFormat::BC6H_RGB:
	case E_ColorFormat::BC6H_RGB_Signed:

	case E_ColorFormat::BC7_RGBA:
	case E_ColorFormat::BC7_SRGBA:
		return 16u;

	default:
		ET_ASSERT(false, "unhandled color format");
		return 0u;
	}
}

//--------------------------------
// TextureFormat::GetCompressedSize
//
// Size in bytes of a (single level) texture
//
size_t TextureFormat::GetCompressedSize(uint32 const width, uint32 const height, E_ColorFormat const storageFormat)
{
	return static_cast<size_t>((width * height) / TextureFormat::s_BlockPixelCount) * static_cast<size_t>(GetBlockByteCount(storageFormat));
}

//------------------------------
// TextureFormat::GetChannelCount
//
uint8 TextureFormat::GetChannelCount(E_ColorFormat const format)
{
	switch (format)
	{
	case E_ColorFormat::Red:

	case E_ColorFormat::R8:

	case E_ColorFormat::BC4_Red:
	case E_ColorFormat::BC4_Red_Signed:
		return 1u;

	case E_ColorFormat::RG:

	case E_ColorFormat::RG8:
	case E_ColorFormat::RG16f:

	case E_ColorFormat::BC5_RG:
	case E_ColorFormat::BC5_RG_Signed:
		return 2u;

	case E_ColorFormat::RGB:
	case E_ColorFormat::BGR:

	case E_ColorFormat::RGB8:
	case E_ColorFormat::SRGB8:
	case E_ColorFormat::RGB16f:

	case E_ColorFormat::BC1_RGB:
	case E_ColorFormat::BC1_SRGB:
	case E_ColorFormat::BC6H_RGB:
	case E_ColorFormat::BC6H_RGB_Signed:
		return 3u;

	case E_ColorFormat::RGBA:
	case E_ColorFormat::BGRA:

	case E_ColorFormat::RGBA8:
	case E_ColorFormat::SRGBA8:
	case E_ColorFormat::RGBA16f:
	case E_ColorFormat::RGBA32f:

	case E_ColorFormat::BC1_RGBA:
	case E_ColorFormat::BC1_SRGBA:
	case E_ColorFormat::BC3_RGBA:
	case E_ColorFormat::BC3_SRGBA:
	case E_ColorFormat::BC7_RGBA:
	case E_ColorFormat::BC7_SRGBA:
		return 4u;

	default:
		ET_ASSERT(false, "unhandled color format");
		return 0u;
	}
}


} // namespace render
} // namespace et
