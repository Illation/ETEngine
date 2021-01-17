#include "stdafx.h"
#include "TextureCompression.h"

#include <bc7enc/rgbcx.h>
#include <bc7enc/bc7enc.h>

#include <ConvectionKernels/ConvectionKernels.h>

#include <EtBuild/EngineVersion.h>

#include <EtCore/IO/BinaryWriter.h>

#include <EtRendering/GlobalRenderingSystems/GlobalRenderingSystems.h>


namespace et {
namespace pl {


//=====================
// Texture Compression
//=====================


// reflection
RTTR_REGISTRATION
{
	rttr::registration::enumeration<TextureCompression::E_Quality>("Compression Quality") (
		rttr::value("Low", TextureCompression::E_Quality::Low),
		rttr::value("Medium", TextureCompression::E_Quality::Medium),
		rttr::value("High", TextureCompression::E_Quality::High),
		rttr::value("Ultra", TextureCompression::E_Quality::Ultra));

	rttr::registration::enumeration<TextureCompression::E_Setting>("Compression Setting") (
		rttr::value("Default", TextureCompression::E_Setting::Default),
		rttr::value("NormalMap", TextureCompression::E_Setting::NormalMap),
		rttr::value("GrayScale", TextureCompression::E_Setting::GrayScale),
		rttr::value("DisplacementMap", TextureCompression::E_Setting::DisplacementMap),
		rttr::value("VectorDisplacementMap", TextureCompression::E_Setting::VectorDisplacementMap),
		rttr::value("HDR", TextureCompression::E_Setting::HDR),
		rttr::value("UI", TextureCompression::E_Setting::UI),
		rttr::value("Alpha", TextureCompression::E_Setting::Alpha),
		rttr::value("SdfFont", TextureCompression::E_Setting::SdfFont),
		rttr::value("BC7", TextureCompression::E_Setting::BC7));
}


//--------------------------------------
// TextureCompression::LoadFromMemory
//
bool TextureCompression::WriteTextureFile(std::vector<uint8>& outFileData,
	RasterImage& source,
	E_Setting const compressionSetting,
	E_Quality const compressionQuality,
	bool const supportsAlpha,
	render::TextureFile::E_Srgb const srgb,
	uint16 const maxSize,
	bool const forceResolution,
	bool const useMipMaps)
{
	uint32 width = source.GetWidth();
	uint32 height = source.GetHeight();

	// settings
	//-----------
	render::E_ColorFormat const storageFormat = GetOutputFormat(compressionSetting, supportsAlpha, srgb != render::TextureFile::E_Srgb::None);
	bool const requiresCompression = render::TextureFile::IsCompressedFormat(storageFormat);
	uint8 const requiredChannels = GetInputChannelCount(storageFormat);

	// resize the texture to an appropriate format
	//----------------------------------------------
	if (!(forceResolution))
	{
		uint32 const size = GetPow2Size(width, height, maxSize, false);
		if ((width != size) || (height != size))
		{
			source.Resize(size, size);
			width = source.GetWidth();
			height = source.GetHeight();
		}
	}
	else
	{
		if (requiresCompression)
		{
			ET_ASSERT(false, "texture forces resolution but requires compression");
			return false;
		}
	}

	// mip maps
	//-----------
	if (useMipMaps)
	{
		source.GenerateMipChain(requiresCompression ? 4u : 1u);
	}

	uint8 const mipCount = source.GetMipLevelCount();

	// precalculate buffer size
	//--------------------------
	// this currently does not take cube maps or 3D textures into account

	size_t mipSize = 0;
	if (requiresCompression)
	{
		mipSize = render::TextureFile::GetCompressedSize(width, height, storageFormat);
	}
	else
	{
		mipSize = static_cast<size_t>(source.GetByteCount());
	}

	size_t bufferSize = mipSize;
	for (uint8 mipIdx = 0u; mipIdx < mipCount; ++mipIdx)
	{
		mipSize /= 4u;
		bufferSize += mipSize;
	}

	// init binary writer
	//--------------------
	core::BinaryWriter binWriter(outFileData);
	binWriter.FormatBuffer(render::TextureFile::s_Header.size() +
		build::Version::s_Name.size() + 1u +
		sizeof(render::E_TextureType) +
		sizeof(uint16) + // width 
		sizeof(uint16) + // height 
		sizeof(uint16) + // layers 
		sizeof(uint8) + // mip count
		sizeof(render::E_ColorFormat) + // gpu storage format
		sizeof(render::E_ColorFormat) + // layout
		sizeof(render::E_DataType) +
		bufferSize);

	// write header
	//--------------
	binWriter.WriteString(render::TextureFile::s_Header);
	binWriter.WriteNullString(build::Version::s_Name);

	binWriter.Write(render::E_TextureType::Texture2D); // only supported format for now

	binWriter.Write(static_cast<uint16>(width));
	binWriter.Write(static_cast<uint16>(height));
	binWriter.Write<uint16>(1u);
	binWriter.Write(mipCount);

	binWriter.Write(storageFormat);

	if (requiresCompression)
	{
		binWriter.Write(render::E_DataType::Invalid);
		binWriter.Write(render::E_ColorFormat::Invalid);
	}
	else
	{
		binWriter.Write(render::E_DataType::UByte);
		switch (requiredChannels)
		{
		case 4u:
			binWriter.Write(render::E_ColorFormat::BGRA);
			source.Swizzle(2u, 1u, 0u, 3u);
			break;

		case 3u:
			binWriter.Write(render::E_ColorFormat::BGR);
			source.Swizzle(2u, 1u, 0u, 3u);
			break;

		case 2u:
			binWriter.Write(render::E_ColorFormat::RG);
			break;

		case 1u:
			binWriter.Write(render::E_ColorFormat::Red);
			break;

		default:
			ET_ASSERT(false, "unexpected channel count");
			return false;
		}
	}

	// write image data per level
	//----------------------------
	RasterImage const* mipImage = &source;
	int32 mipLevel = 0;
	while (mipImage != nullptr)
	{
		if (requiresCompression)
		{
			std::vector<uint8> compressedData;
			if (!CompressImage(*mipImage, storageFormat, compressionQuality, compressedData))
			{
				ET_ASSERT(false, "Failed to compress image");
				return false;
			}

			binWriter.WriteData(compressedData.data(), compressedData.size());
		}
		else // write bitmap - in the future we might want to store in the usual file formats here
		{
			// for runtime data generation we would convert to a more convenient to load layout, and potentially remove certain channels
			switch (requiredChannels)
			{
			case 4u:
				binWriter.WriteData(reinterpret_cast<uint8 const*>(mipImage->GetPixels()), static_cast<size_t>(mipImage->GetByteCount()));
				break;

			case 3u:
			case 2u:
			case 1u:
			{
				std::vector<uint8> bitmap = mipImage->GetPixels(requiredChannels);
				binWriter.WriteData(bitmap.data(), bitmap.size());
			}
			break;

			default:
				ET_ASSERT(false, "unexpected channel count");
				return false;
			}
		}

		// "recurse"
		++mipLevel;
		mipImage = mipImage->GetMipChild();
	}

	return true;
}

//-----------------------------------
// TextureCompression::GetPow2Size
//
// Return the nearest power of two of the larger edge, clamped to the max size and adjusted by graphics settings
//
uint32 TextureCompression::GetPow2Size(uint32 const width, uint32 const height, uint16 const maxSize, bool adjustByGraphicsSettings)
{
	uint32 size = std::max(width, height);
	if (maxSize != 0u)
	{
		size = std::min(static_cast<uint32>(maxSize), size);
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
// TextureCompression::GetOutputFormat
//
render::E_ColorFormat TextureCompression::GetOutputFormat(E_Setting const setting, bool const supportAlpha, bool const useSrgb)
{
	switch (setting)
	{
	case E_Setting::Default:
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

	case E_Setting::NormalMap:
		ET_ASSERT(!supportAlpha);
		ET_ASSERT(!useSrgb);
		return render::E_ColorFormat::BC5_RG;

	case E_Setting::GrayScale:
		ET_ASSERT(!supportAlpha);
		if (useSrgb)
		{
			return render::E_ColorFormat::SRGB8;
		}

		return render::E_ColorFormat::R8;

	case E_Setting::DisplacementMap:
		ET_ASSERT(!supportAlpha);
		ET_ASSERT(!useSrgb);
		return render::E_ColorFormat::R8; // in the future maybe also support R16, but there is no point now because we load 8 bits

	case E_Setting::VectorDisplacementMap:
		ET_ASSERT(!supportAlpha);
		ET_ASSERT(!useSrgb);
		return render::E_ColorFormat::RGB8;

	case E_Setting::HDR:
		ET_ASSERT(!supportAlpha);
		ET_ASSERT(!useSrgb);
		return render::E_ColorFormat::RGB16f;

	case E_Setting::UI:
		if (useSrgb)
		{
			if (supportAlpha)
			{
				return render::E_ColorFormat::SRGBA8;
			}

			return render::E_ColorFormat::SRGB8;
		}

		if (supportAlpha)
		{
			return render::E_ColorFormat::RGBA8;
		}

		return render::E_ColorFormat::RGB8;

	case E_Setting::Alpha:
		ET_ASSERT(!useSrgb);
		return render::E_ColorFormat::BC4_Red; // signed?

	case E_Setting::SdfFont:
		ET_ASSERT(!useSrgb);
		ET_ASSERT(supportAlpha);
		return render::E_ColorFormat::RGBA8;

	case E_Setting::BC7:
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
// TextureCompression::GetInputChannelCount
//
uint8 TextureCompression::GetInputChannelCount(render::E_ColorFormat const format)
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

//-------------------------------------
// TextureCompression::CompressImage
//
// #todo: in the future we should really be using multiple threads
//
bool TextureCompression::CompressImage(RasterImage const& image,
	render::E_ColorFormat const format,
	E_Quality const compressionQuality,
	std::vector<uint8>& outData)
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

	uint32 qualityLevel;
	switch (compressionQuality)
	{
	case E_Quality::Low:
		qualityLevel = 0;
		break;

	case E_Quality::Medium:
		qualityLevel = 5;
		break;

	case E_Quality::High:
		qualityLevel = 11;
		break;

	case E_Quality::Ultra:
		qualityLevel = 18;
		break;
	}

	uint8 const byteCount = render::TextureFile::GetBlockByteCount(format);
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
					rgbcx::encode_bc1(qualityLevel, compressedBlock, &blockPixels[0].m_Channels[0], false, true);
					break;

					//case render::E_ColorFormat::BC1_RGBA:
					//case render::E_ColorFormat::BC1_SRGBA:
					//	rgbcx::encode_bc1(qualityLevel, compressedBlock, &blockPixels[0].m_Channels[0], true, false);
					//	break;

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
			switch (compressionQuality)
			{
			case E_Quality::Low:
				bc7PackParams.m_uber_level = 0;
				break;

			case E_Quality::Medium:
				bc7PackParams.m_uber_level = 2;
				break;

			case E_Quality::High:
				bc7PackParams.m_uber_level = 3;
				break;

			case E_Quality::Ultra:
				bc7PackParams.m_uber_level = 4;
				break;
			}

			bc7PackParams.m_max_partitions_mode = BC7ENC_MAX_PARTITIONS1 /
				(static_cast<uint8>(E_Quality::Ultra) - static_cast<uint8>(compressionQuality) + 1u);

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

