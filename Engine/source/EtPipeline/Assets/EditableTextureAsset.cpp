#include "stdafx.h"
#include "EditableTextureAsset.h"

#include <stb/stb_image.h>
#include <stb/stb_image_resize.h>

#include <EtRendering/GlobalRenderingSystems/GlobalRenderingSystems.h>


namespace et {
namespace pl {


//========================
// Editable Texture Asset
//========================


// reflection
RTTR_REGISTRATION
{
	rttr::registration::enumeration<EditableTextureAsset::E_CompressionQuality>("E_CompressionQuality") (
		rttr::value("Low", EditableTextureAsset::E_CompressionQuality::Low),
		rttr::value("Medium", EditableTextureAsset::E_CompressionQuality::Medium),
		rttr::value("High", EditableTextureAsset::E_CompressionQuality::High),
		rttr::value("Ultra", EditableTextureAsset::E_CompressionQuality::Ultra));

	BEGIN_REGISTER_CLASS(EditableTextureAsset, "editable texture asset")
		.property("sRGB setting", &EditableTextureAsset::m_Srgb)
		.property("supports alpha", &EditableTextureAsset::m_SupportAlpha)
		.property("max size", &EditableTextureAsset::m_MaxSize)
		.property("compression setting", &EditableTextureAsset::m_CompressionSetting)
		.property("compression quality", &EditableTextureAsset::m_CompressionQuality)
	END_REGISTER_CLASS_POLYMORPHIC(EditableTextureAsset, EditorAssetBase);
}
DEFINE_FORCED_LINKING(EditableTextureAsset) // force the asset class to be linked as it is only used in reflection


//--------------------------------------
// EditableTextureAsset::LoadFromMemory
//
bool EditableTextureAsset::LoadFromMemory(std::vector<uint8> const& data)
{
	render::TextureAsset const* const textureAsset = static_cast<render::TextureAsset const*>(m_Asset);

	ET_ASSERT((m_Srgb != render::E_SrgbSetting::None) == textureAsset->m_UseSrgb);
	render::E_ColorFormat outputFormat = GetOutputFormat(m_CompressionSetting, m_SupportAlpha, m_Srgb != render::E_SrgbSetting::None);
	bool const requiresCompression = render::RequiresCompression(m_CompressionSetting);
	int32 channels = requiresCompression ? 4u : GetInputChannelCount(outputFormat);

	// check image format
	stbi_set_flip_vertically_on_load(false);
	int32 width = 0;
	int32 height = 0;
	int32 fileChannels = 0;

	// option to load 16 bit texture
	uint8* bits = stbi_load_from_memory(data.data(), static_cast<int32>(data.size()), &width, &height, &fileChannels, channels);

	if (channels == 0)
	{
		channels = fileChannels;
	}

	if (bits == nullptr)
	{
		LOG("TextureAsset::LoadFromMemory > Failed to load texture bytes from data!", core::LogLevel::Warning);
		return false;
	}

	if ((width == 0) || (height == 0))
	{
		LOG("TextureAsset::LoadFromMemory > Image is too small to display!", core::LogLevel::Warning);
		stbi_image_free(bits);
		return false;
	}

	// resize the texture to an appropriate format
	int32 const size = GetPow2Size(width, height, true);
	if (!(textureAsset->m_ForceResolution))
	{
		if ((width != size) || (height != size))
		{
			// resize
			uint8* outBits = new uint8[size * size * channels];

			stbir_resize_uint8(bits, width, height, 0, outBits, size, size, 0, channels);

			stbi_image_free(bits);
			bits = outBits;
			width = size;
			height = size;
		}
	}
	else
	{
		ET_ASSERT(!requiresCompression, "texture '%s' forces resolution but requires compression", m_Id.ToStringDbg());
	}

	if (requiresCompression)
	{
		uint32 const blocksX = width / 4;
		uint32 const blocksY = height / 4;

		std::vector<T_Block8> packedImage8(blocksX * blocksY);
		std::vector<Block16> packedImage16(blocksX * blocksY);
	}

	// convert data type
	// check number of channels
	render::E_ColorFormat layout;
	render::E_ColorFormat storageFormat;
	switch (channels)
	{
	case 1:
		layout = render::E_ColorFormat::Red;
		ET_ASSERT(m_Srgb != render::E_SrgbSetting::OnLoad);
		storageFormat = render::E_ColorFormat::R8;
		break;

	case 2:
		layout = render::E_ColorFormat::RG;
		ET_ASSERT(m_Srgb != render::E_SrgbSetting::OnLoad);
		storageFormat = render::E_ColorFormat::RG8;
		break;

	case 3:
		layout = render::E_ColorFormat::RGB;
		if (m_Srgb == render::E_SrgbSetting::OnLoad)
		{
			storageFormat = render::E_ColorFormat::SRGB8;
		}
		else
		{
			storageFormat = render::E_ColorFormat::RGB8;
		}
		break;

	case 4:
		layout = render::E_ColorFormat::RGBA;
		if (m_Srgb == render::E_SrgbSetting::OnLoad)
		{
			storageFormat = render::E_ColorFormat::SRGBA8;
		}
		else
		{
			storageFormat = render::E_ColorFormat::RGBA8;
		}
		break;

	default:
		ET_ASSERT(false, "unhandled texture channel count");
		stbi_image_free(bits);
		return false;
	}

	//Upload to GPU
	render::TextureData* const texture = new render::TextureData(storageFormat, ivec2(width, height));
	texture->UploadData(reinterpret_cast<void const*>(bits), layout, render::E_DataType::UByte);

	stbi_image_free(bits);
	bits = nullptr;

	texture->SetParameters(textureAsset->m_Parameters);
	texture->CreateHandle();

	SetData(texture);
	return true;
}

//-----------------------------------
// EditableTextureAsset::GetPow2Size
//
// Return the nearest power of two of the larger edge, clamped to the max size and adjusted by graphics settings
//
int32 EditableTextureAsset::GetPow2Size(int32 const width, int32 const height, bool adjustByGraphicsSettings) const
{
	int32 size = std::max(width, height);
	if (m_MaxSize != 0u)
	{
		size = std::min(static_cast<int32>(m_MaxSize), size);
	}

	if (adjustByGraphicsSettings)
	{
		render::GraphicsSettings const& graphicsSettings = render::RenderingSystems::Instance()->GetGraphicsSettings();
		if (!math::nearEquals(graphicsSettings.TextureScaleFactor, 1.f))
		{
			size = static_cast<int32>(static_cast<float>(size) * graphicsSettings.TextureScaleFactor);
		}
	}

	int32 power = 1;
	while (power < size)
	{
		power *= 2;
	}

	return power;
}

//---------------------------------------
// EditableTextureAsset::GetOutputFormat
//
render::E_ColorFormat EditableTextureAsset::GetOutputFormat(render::E_CompressionSetting const setting, bool const supportAlpha, bool const useSrgb) const
{
	switch (setting)
	{
	case render::E_CompressionSetting::Default:
		if (useSrgb)
		{
			if (supportAlpha)
			{
				return render::E_ColorFormat::BC3_SRGBA;
			}

			return render::E_ColorFormat::BC1_SRGB; 
		}

		if (supportAlpha)
		{
			return render::E_ColorFormat::BC3_RGBA;
		}

		return render::E_ColorFormat::BC1_RGB; // in the future we might allow for one bit alpha

	case render::E_CompressionSetting::NormalMap:
		ET_ASSERT(!supportAlpha);
		ET_ASSERT(!useSrgb);
		return render::E_ColorFormat::BC5_RG;

	case render::E_CompressionSetting::GrayScale:
		ET_ASSERT(!supportAlpha);
		if (useSrgb)
		{
			return render::E_ColorFormat::SRGB8;
		}

		return render::E_ColorFormat::R8;

	case render::E_CompressionSetting::DisplacementMap:
		ET_ASSERT(!supportAlpha);
		ET_ASSERT(!useSrgb);
		return render::E_ColorFormat::R8; // in the future maybe also support R16, but there is no point now because we load 8 bits

	case render::E_CompressionSetting::VectorDisplacementMap:
		ET_ASSERT(!supportAlpha);
		ET_ASSERT(!useSrgb);
		return render::E_ColorFormat::RGB8;

	case render::E_CompressionSetting::HDR:
		ET_ASSERT(!supportAlpha);
		ET_ASSERT(!useSrgb);
		return render::E_ColorFormat::RGB16f;

	case render::E_CompressionSetting::UI:
		ET_ASSERT(supportAlpha);
		if (useSrgb)
		{
			return render::E_ColorFormat::SRGBA8;
		}

		return render::E_ColorFormat::RGBA8;

	case render::E_CompressionSetting::Alpha:
		ET_ASSERT(!useSrgb);
		return render::E_ColorFormat::BC4_Red; // signed?

	case render::E_CompressionSetting::SdfFont:
		ET_ASSERT(!useSrgb);
		ET_ASSERT(supportAlpha);
		return render::E_ColorFormat::RGBA8; 

	case render::E_CompressionSetting::BC7:
		ET_ASSERT(supportAlpha);
		if (useSrgb)
		{
			return render::E_ColorFormat::BC7_SRGBA;
		}

		return render::E_ColorFormat::BC7_RGBA;
	}
}

//--------------------------------------------
// EditableTextureAsset::GetInputChannelCount
//
uint8 EditableTextureAsset::GetInputChannelCount(render::E_ColorFormat const format) const
{
	switch (format)
	{
	case render::E_ColorFormat::R8:
	case render::E_ColorFormat::BC4_Red:
		return 1u;

	case render::E_ColorFormat::BC5_RG:
		return 2u;

	case render::E_ColorFormat::RGB8:
	case render::E_ColorFormat::SRGB8:
	case render::E_ColorFormat::RGB16f:
	case render::E_ColorFormat::BC1_RGB:
	case render::E_ColorFormat::BC1_SRGB:
		return 3u;

	case render::E_ColorFormat::RGBA8:
	case render::E_ColorFormat::SRGBA8:
	case render::E_ColorFormat::BC3_RGBA:
	case render::E_ColorFormat::BC3_SRGBA:
	case render::E_ColorFormat::BC7_RGBA:
	case render::E_ColorFormat::BC7_SRGBA:
		return 4u;

	default: 
		ET_ASSERT(false, "unhandled color format");
		return 0u;
	}
}

//-----------------------------------
// EditableTextureAsset::GetBlock4x4
//
// Get a block for compression from bitmap. X and Y are block coordinates, not bitmap coordinates
// Assumes input pixels are 4 channels
// outBlock should be an array of 16 elements
//
void EditableTextureAsset::GetBlock4x4(uint8 const* const pixels, 
	int32 const width, 
	int32 const height, 
	uint16 const blockX, 
	uint16 const blockY, 
	ColorU8* const outBlock)
{
	UNUSED(width);
	UNUSED(height);
	ET_ASSERT((blockX * 4 + 4) <= width);
	ET_ASSERT((blockY * 4 + 4) <= height);

	for (uint32_t y = 0; y < 4; y++)
	{
		memcpy(reinterpret_cast<void*>(outBlock + y * 4), 
			reinterpret_cast<void const*>(pixels[(blockY * 4 + y) + (blockX * 4)]), 
			4 * sizeof(ColorU8));
	}
}


} // namespace pl
} // namespace et
