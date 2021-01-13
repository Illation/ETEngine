#include "stdafx.h"
#include "EditableTextureAsset.h"

#include <stb/stb_image.h>

#include <bc7enc/rgbcx.h>
#include <bc7enc/bc7enc.h>
//#include "../bc7enc/bc7enc/rgbcx.h"
//#include "../bc7enc/bc7enc/bc7enc.h"

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

	// settings
	ET_ASSERT((m_Srgb != render::E_SrgbSetting::None) == textureAsset->m_UseSrgb);
	render::E_ColorFormat outputFormat = GetOutputFormat(m_CompressionSetting, m_SupportAlpha, m_Srgb != render::E_SrgbSetting::None);
	bool const requiresCompression = render::RequiresCompression(m_CompressionSetting);

	// Load
	RasterImage image;
	if (!LoadImage(image, data))
	{
		ET_ASSERT(false, "Failed to load image from data");
		return false;
	}

	uint32 width = image.GetWidth();
	uint32 height = image.GetHeight();

	// resize the texture to an appropriate format
	if (!(textureAsset->m_ForceResolution))
	{
		uint32 const size = GetPow2Size(width, height, true);
		if ((width != size) || (height != size))
		{
			image.Resize(size, size);
			width = image.GetWidth();
			height = image.GetHeight();
		}
	}
	else
	{
		ET_ASSERT(!requiresCompression, "texture '%s' forces resolution but requires compression", m_Id.ToStringDbg());
	}

	// generate mipmaps here in the future

	render::TextureData* texture = nullptr;
	if (requiresCompression)
	{
		// #todo: handle mip map levels

		std::vector<uint8> compressedData;
		if (!CompressImage(image, outputFormat, compressedData))
		{
			ET_ASSERT(false, "Failed to compress image");
			return false;
		}

		texture = new render::TextureData(outputFormat, ivec2(static_cast<int32>(width), static_cast<int32>(height)));
		texture->UploadCompressed(reinterpret_cast<void const*>(compressedData.data()), compressedData.size());

		render::TextureParameters params = textureAsset->m_Parameters;
		params.genMipMaps = false; // not supported for compressed textures

		texture->SetParameters(params);
	}
	else
	{
		// for runtime data generation we would convert to a more convenient to load layout, and potentially remove certain channels
		// image.swizzle(2u, 1u, 0u, 3u);
		// image.GetPixels(GetInputChannelCount(outputFormat));

		// temp
		render::E_ColorFormat const layout = render::E_ColorFormat::RGBA;
		render::E_ColorFormat const storageFormat = 
			(m_Srgb == render::E_SrgbSetting::OnLoad) ? render::E_ColorFormat::SRGBA8 : render::E_ColorFormat::RGBA8;

		//Upload to GPU
		texture = new render::TextureData(storageFormat, ivec2(static_cast<int32>(width), static_cast<int32>(height)));
		texture->UploadData(reinterpret_cast<void const*>(image.GetPixels()), layout, render::E_DataType::UByte);

		texture->SetParameters(textureAsset->m_Parameters);
	}

	texture->CreateHandle();

	// done
	SetData(texture);
	return true;
}

//---------------------------------
// EditableTextureAsset::LoadImage
//
bool EditableTextureAsset::LoadImage(RasterImage& image, std::vector<uint8> const& data) const
{
	// check image format
	stbi_set_flip_vertically_on_load(false);
	int32 width = 0;
	int32 height = 0;
	int32 fileChannels = 0;

	// option to load 16 bit texture
	uint8* const pixels = stbi_load_from_memory(data.data(), 
		static_cast<int32>(data.size()), 
		&width, 
		&height, 
		&fileChannels, 
		static_cast<int32>(RasterImage::s_NumChannels));
	if ((pixels == nullptr) || (width == 0) || (height == 0))
	{
		return false;
	}

	image.SetSize(static_cast<uint32>(width), static_cast<uint32>(height));
	image.AllocatePixels();
	image.SetPixels(reinterpret_cast<RasterImage::ColorU8 const*>(pixels));

	stbi_image_free(pixels);
	 
	return true;
}

//-----------------------------------
// EditableTextureAsset::GetPow2Size
//
// Return the nearest power of two of the larger edge, clamped to the max size and adjusted by graphics settings
//
uint32 EditableTextureAsset::GetPow2Size(uint32 const width, uint32 const height, bool adjustByGraphicsSettings) const
{
	uint32 size = std::max(width, height);
	if (m_MaxSize != 0u)
	{
		size = std::min(static_cast<uint32>(m_MaxSize), size);
	}

	if (adjustByGraphicsSettings)
	{
		render::GraphicsSettings const& graphicsSettings = render::RenderingSystems::Instance()->GetGraphicsSettings();
		if (!math::nearEquals(graphicsSettings.TextureScaleFactor, 1.f))
		{
			size = static_cast<uint32>(static_cast<float>(size) * graphicsSettings.TextureScaleFactor);
		}
	}

	return RasterImage::GetClosestPowerOf2(size);
}

//---------------------------------------
// EditableTextureAsset::GetOutputFormat
//
render::E_ColorFormat EditableTextureAsset::GetOutputFormat(render::E_CompressionSetting const setting, bool const supportAlpha, bool const useSrgb) const
{
	// hack for now because normal maps are broken
	render::E_CompressionSetting const settingOverride = (setting == render::E_CompressionSetting::NormalMap) ? 
		render::E_CompressionSetting::Default : setting;

	switch (settingOverride)
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

	ET_ASSERT(false, "unhandled compression setting");
	return render::E_ColorFormat::Invalid;
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

//-----------------------------------------
// EditableTextureAsset::GetBlockByteCount
//
uint8 EditableTextureAsset::GetBlockByteCount(render::E_ColorFormat const format) const
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

//-------------------------------------
// EditableTextureAsset::CompressImage
//
// #todo: in the future we should really be using multiple threads
//
bool EditableTextureAsset::CompressImage(RasterImage const& image, render::E_ColorFormat const format, std::vector<uint8>& outData) const
{
	//---------------------------------------------
	// 8 byte compressed block
	struct Block8
	{
		uint64 m_Val;
	};

	//---------------------------------------------
	// 16 byte compressed block
	struct Block16
	{
		uint64 m_Val[2];
	};

	static uint8 const s_Bc45Channel0 = 0u;
	static uint8 const s_Bc45Channel1 = 1u;

	uint32 const blocksX = image.GetWidth() / 4;
	uint32 const blocksY = image.GetHeight() / 4;

	bool use3Color;
	uint32 qualityLevel;
	switch (m_CompressionQuality)
	{
	case E_CompressionQuality::Low:
		use3Color = false;
		qualityLevel = 0;
		break;

	case E_CompressionQuality::Medium:
		use3Color = false;
		qualityLevel = 5;
		break;

	case E_CompressionQuality::High:
		use3Color = true;
		qualityLevel = 11;
		break;

	case E_CompressionQuality::Ultra:
		use3Color = true;
		qualityLevel = 18;
		break;
	}

	uint8 const byteCount = GetBlockByteCount(format);
	switch (byteCount)
	{
	case 8u:
	{
		rgbcx::init(rgbcx::bc1_approx_mode::cBC1Ideal); // in the future we can make this platform dependent

		outData.resize(blocksX * blocksY * sizeof(Block8));
		Block8* const packedImage = reinterpret_cast<Block8*>(outData.data());

		for (uint32 blockX = 0u; blockX < blocksX; ++blockX)
		{
			for (uint32 blockY = 0u; blockY < blocksY; ++blockY)
			{
				RasterImage::ColorU8 blockPixels[16u];
				image.GetBlock(blockX, blockY, 4u, 4u, blockPixels);

				Block8* compressedBlock = &packedImage[blockX + blockY * blocksX];

				switch (format)
				{
				case render::E_ColorFormat::BC1_RGB:
				case render::E_ColorFormat::BC1_SRGB:
				//case render::E_ColorFormat::BC1_RGBA:
				//case render::E_ColorFormat::BC1_SRGBA:
					rgbcx::encode_bc1(qualityLevel, compressedBlock, &blockPixels[0].m_Channels[0], use3Color, !m_SupportAlpha);
					break;

				case render::E_ColorFormat::BC4_Red:
					//case render::E_ColorFormat::BC4_Red_Signed:
					rgbcx::encode_bc4(compressedBlock, &blockPixels[0].m_Channels[s_Bc45Channel0], RasterImage::s_NumChannels);
					break;

				default:
					ET_ASSERT(false, "unhandled 8 byte block type");
					return false;
				}
			}
		}
	}
	break;

	case 16u:
	{
		bc7enc_compress_block_params bc7PackParams;

		switch (format)
		{
		case render::E_ColorFormat::BC7_RGBA:
		case render::E_ColorFormat::BC7_SRGBA:
			bc7enc_compress_block_init();

			bc7enc_compress_block_params_init(&bc7PackParams);
			switch (m_CompressionQuality)
			{
			case E_CompressionQuality::Low:
				bc7PackParams.m_uber_level = 0;
				break;

			case E_CompressionQuality::Medium:
				bc7PackParams.m_uber_level = 2;
				break;

			case E_CompressionQuality::High:
				bc7PackParams.m_uber_level = 3;
				break;

			case E_CompressionQuality::Ultra:
				bc7PackParams.m_uber_level = 4;
				break;
			}

			bc7PackParams.m_max_partitions_mode = BC7ENC_MAX_PARTITIONS1 /
				(static_cast<uint8>(E_CompressionQuality::Ultra) - static_cast<uint8>(m_CompressionQuality) + 1u);

			break;

		default:
			rgbcx::init(); // don't care what mode since we're not compresssing to BC1
			break;
		}

		outData.resize(blocksX * blocksY * sizeof(Block16));
		Block16* const packedImage = reinterpret_cast<Block16*>(outData.data());


		for (uint32 blockX = 0u; blockX < blocksX; ++blockX)
		{
			for (uint32 blockY = 0u; blockY < blocksY; ++blockY)
			{
				RasterImage::ColorU8 blockPixels[16u];
				image.GetBlock(blockX, blockY, 4u, 4u, blockPixels);

				Block16* compressedBlock = &packedImage[blockX + blockY * blocksX];

				switch (format)
				{
				case render::E_ColorFormat::BC3_RGBA:
				case render::E_ColorFormat::BC3_SRGBA:
					rgbcx::encode_bc3(qualityLevel, compressedBlock, &blockPixels[0].m_Channels[0]);
					break;

				case render::E_ColorFormat::BC5_RG:
					//case render::E_ColorFormat::BC5_RG_Signed:
					rgbcx::encode_bc5(compressedBlock, &blockPixels[0].m_Channels[0], s_Bc45Channel0, s_Bc45Channel1, RasterImage::s_NumChannels);
					break;

				case render::E_ColorFormat::BC7_RGBA:
				case render::E_ColorFormat::BC7_SRGBA:
					bc7enc_compress_block(compressedBlock, reinterpret_cast<void const*>(blockPixels), &bc7PackParams);
					break;

				default:
					ET_ASSERT(false, "unhandled 16 byte block type");
					return false;
				}
			}
		}
	}
	break;

	default:
		ET_ASSERT(false, "unhandled block byte count");
		return false;
	}

	return true;
}


} // namespace pl
} // namespace et
