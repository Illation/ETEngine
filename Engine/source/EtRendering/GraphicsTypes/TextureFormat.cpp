#include "stdafx.h"
#include "TextureFormat.h"
#include <EtCore/Reflection/ReflectionUtil.h>


namespace et {
namespace render {


// reflection
RTTR_REGISTRATION
{
	using namespace rttr;

	registration::enumeration<TextureFile::E_Srgb>("E_SrgbSetting") (
		value("None", TextureFile::E_Srgb::None),
		value("OnLoad", TextureFile::E_Srgb::OnLoad));
}

DEFINE_FORCED_LINKING(TextureFile) // force the linker to include this unit

// static
std::string const TextureFile::s_Header("ETTEX");
size_t const TextureFile::s_BlockPixelCount = 16u;


//---------------------------------
// TextureFile::IsCompressedFormat
//
bool TextureFile::IsCompressedFormat(E_ColorFormat const format)
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

	//case E_ColorFormat::BC6H_RGB_Signed:
	//case E_ColorFormat::BC6H_RGB_Unsigned:

	case E_ColorFormat::BC7_RGBA:
	case E_ColorFormat::BC7_SRGBA:
		return true;
	}

	return false;
}

//----------------------------------
// TextureFile::GetBlockByteCount
//
uint8 TextureFile::GetBlockByteCount(render::E_ColorFormat const format)
{
	switch (format)
	{
	case render::E_ColorFormat::BC1_RGB:
	case render::E_ColorFormat::BC1_RGBA:
	case render::E_ColorFormat::BC1_SRGB:
	case render::E_ColorFormat::BC1_SRGBA:

	case render::E_ColorFormat::BC4_Red:
	case render::E_ColorFormat::BC4_Red_Signed:
		return 8u;

	case render::E_ColorFormat::BC3_RGBA:
	case render::E_ColorFormat::BC3_SRGBA:

	case render::E_ColorFormat::BC5_RG:
	case render::E_ColorFormat::BC5_RG_Signed:

	case render::E_ColorFormat::BC7_RGBA:
	case render::E_ColorFormat::BC7_SRGBA:
		return 16u;

	default:
		ET_ASSERT(false, "unhandled color format");
		return 0u;
	}
}

//--------------------------------
// TextureFile::GetCompressedSize
//
// Size in bytes of a (single level) texture
//
size_t TextureFile::GetCompressedSize(uint32 const width, uint32 const height, E_ColorFormat const storageFormat)
{
	return static_cast<size_t>((width * height) / TextureFile::s_BlockPixelCount) * static_cast<size_t>(GetBlockByteCount(storageFormat));
}


} // namespace render
} // namespace et
