#include "stdafx.h"
#include "TextureCompression.h"

#include <bc7enc/rgbcx.h>
#include <bc7enc/bc7enc.h>

#include <ConvectionKernels/ConvectionKernels.h>

#include <EtBuild/EngineVersion.h>

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

size_t const TextureCompression::s_PixelBufferSize = 16u * 4u * 2u; // numPixels * numChannels * biggestSourceDataSize (16bit int or half float)


//----------------------------------------
// TextureCompression::WriteTextureHeader
//
// Also formats the buffer to the correct size
//
void TextureCompression::WriteTextureHeader(core::BinaryWriter& binWriter,
	size_t const bufferSize,
	render::E_TextureType const textureType,
	uint32 const width,
	uint32 const height,
	uint8 const mipCount,
	render::E_ColorFormat const storageFormat)
{
	binWriter.FormatBuffer(render::TextureFormat::s_Header.size() +
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
	binWriter.WriteString(render::TextureFormat::s_Header);
	binWriter.WriteNullString(build::Version::s_Name);

	binWriter.Write(textureType); 

	binWriter.Write(static_cast<uint16>(width));
	binWriter.Write(static_cast<uint16>(height));
	binWriter.Write<uint16>(1u);
	binWriter.Write(mipCount);

	binWriter.Write(storageFormat);
}

//--------------------------------------
// TextureCompression::LoadFromMemory
//
bool TextureCompression::WriteTextureFile(std::vector<uint8>& outFileData,
	RasterImage& source,
	E_Setting const compressionSetting,
	E_Quality const compressionQuality,
	bool const supportsAlpha,
	render::TextureFormat::E_Srgb const srgb,
	uint16 const maxSize,
	bool const forceResolution,
	bool const useMipMaps)
{
	uint32 width = source.GetWidth();
	uint32 height = source.GetHeight();

	// settings
	//-----------
	render::E_ColorFormat const storageFormat = GetOutputFormat(compressionSetting, supportsAlpha, srgb != render::TextureFormat::E_Srgb::None);
	bool const requiresCompression = render::TextureFormat::IsCompressedFormat(storageFormat);
	uint8 const requiredChannels = render::TextureFormat::GetChannelCount(storageFormat);

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
	// this is only for 2D textures - cubemap implementation in "EditableEnvironmentMap.cpp"

	size_t mipSize = 0;
	if (requiresCompression)
	{
		mipSize = render::TextureFormat::GetCompressedSize(width, height, storageFormat);
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

	// header
	//--------
	core::BinaryWriter binWriter(outFileData);
	WriteTextureHeader(binWriter, bufferSize, render::E_TextureType::Texture2D, width, height, mipCount, storageFormat);

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
	while (mipImage != nullptr)
	{
		if (requiresCompression)
		{
			std::vector<uint8> compressedData;
			if (!CompressImageU8(*mipImage, storageFormat, compressionQuality, compressedData))
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

	case E_Setting::CompressedHDR:
		ET_ASSERT(!useSrgb);
		ET_ASSERT(!supportAlpha);
		return render::E_ColorFormat::BC6H_RGB;

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

//-------------------------------------
// TextureCompression::CompressImage
//
// #todo: in the future we should really be using multiple threads
//
bool TextureCompression::CompressImage(void const* const sourceData,
	uint32 const blockCount,
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

	auto getS3TCQuality = [compressionQuality]() -> uint32
		{
			switch (compressionQuality)
			{
			case E_Quality::Low:
				return 0u;

			case E_Quality::Medium:
				return 5u;

			case E_Quality::High:
				return 11u;

			case E_Quality::Ultra:
				return 18u;

			default:
				ET_ASSERT(false, "unhandled compression quality");
				return 0u;
			}
		};

	switch (format)
	{
	case render::E_ColorFormat::BC1_RGB:
	case render::E_ColorFormat::BC1_SRGB:
	{
		rgbcx::init(rgbcx::bc1_approx_mode::cBC1Ideal); // in the future we can make this platform dependent

		outData.resize(blockCount * sizeof(Block8));
		Block8* const packedImage = reinterpret_cast<Block8*>(outData.data());

		uint32 const qualityLevel = getS3TCQuality();
		for (uint32 blockIdx = 0u; blockIdx < blockCount; ++blockIdx)
		{
			uint8 const* const blockPixels = reinterpret_cast<uint8 const*>(sourceData) + (blockIdx * 4u * 16u); // channel count * pixels per block
			Block8* compressedBlock = &packedImage[blockIdx];
			rgbcx::encode_bc1(qualityLevel, compressedBlock, blockPixels, false, true);
		}

		break;
	}

	case render::E_ColorFormat::BC1_RGBA:
	case render::E_ColorFormat::BC1_SRGBA:
	{
		rgbcx::init(rgbcx::bc1_approx_mode::cBC1Ideal); // in the future we can make this platform dependent

		outData.resize(blockCount * sizeof(Block8));
		Block8* const packedImage = reinterpret_cast<Block8*>(outData.data());

		uint32 const qualityLevel = getS3TCQuality();
		for (uint32 blockIdx = 0u; blockIdx < blockCount; ++blockIdx)
		{
			uint8 const* const blockPixels = reinterpret_cast<uint8 const*>(sourceData) + (blockIdx * 4u * 16u); // channel count * pixels per block
			Block8* compressedBlock = &packedImage[blockIdx];
			rgbcx::encode_bc1(qualityLevel, compressedBlock, blockPixels, true, false);
		}

		break;
	}

	case render::E_ColorFormat::BC3_RGBA:
	case render::E_ColorFormat::BC3_SRGBA:
	{
		rgbcx::init(); // don't care what mode since we're not compresssing to BC1

		outData.resize(blockCount * sizeof(Block16));
		Block16* const packedImage = reinterpret_cast<Block16*>(outData.data());

		uint32 const qualityLevel = getS3TCQuality();
		for (uint32 blockIdx = 0u; blockIdx < blockCount; ++blockIdx)
		{
			uint8 const* const blockPixels = reinterpret_cast<uint8 const*>(sourceData) + (blockIdx * 4u * 16u); // channel count * pixels per block
			Block16* compressedBlock = &packedImage[blockIdx];
			rgbcx::encode_bc3(qualityLevel, compressedBlock, blockPixels);
		}

		break;
	}

	case render::E_ColorFormat::BC4_Red:
	//case render::E_ColorFormat::BC4_Red_Signed:
	{
		rgbcx::init(); 

		outData.resize(blockCount * sizeof(Block8));
		Block8* const packedImage = reinterpret_cast<Block8*>(outData.data());

		for (uint32 blockIdx = 0u; blockIdx < blockCount; ++blockIdx)
		{
			uint8 const* const blockPixels = reinterpret_cast<uint8 const*>(sourceData) + (blockIdx * 4u * 16u); // channel count * pixels per block
			Block8* compressedBlock = &packedImage[blockIdx];
			rgbcx::encode_bc4(compressedBlock, blockPixels, RasterImage::s_NumChannels);
		}

		break;
	}

	case render::E_ColorFormat::BC5_RG:
	//case render::E_ColorFormat::BC5_RG_Signed:
	{
		rgbcx::init(); // don't care what mode since we're not compresssing to BC1

		outData.resize(blockCount * sizeof(Block16));
		Block16* const packedImage = reinterpret_cast<Block16*>(outData.data());

		for (uint32 blockIdx = 0u; blockIdx < blockCount; ++blockIdx)
		{
			uint8 const* const blockPixels = reinterpret_cast<uint8 const*>(sourceData) + (blockIdx * 4u * 16u); // channel count * pixels per block
			Block16* compressedBlock = &packedImage[blockIdx];
			rgbcx::encode_bc5(compressedBlock, blockPixels, s_Bc45Channel0, s_Bc45Channel1, RasterImage::s_NumChannels);
		}

		break;
	}

	case render::E_ColorFormat::BC6H_RGB:
	//case render::E_ColorFormat::BC6H_RGB:
	{
		cvtt::Options options;
		switch (compressionQuality)
		{
		case E_Quality::Low:
			options.flags = cvtt::Flags::Fastest;
			options.refineRoundsBC6H = 1;
			options.seedPoints = 1;
			break;

		case E_Quality::Medium:
			options.flags = cvtt::Flags::Default;
			options.refineRoundsBC6H = 2;
			options.seedPoints = 2;
			break;

		case E_Quality::High:
			options.flags = cvtt::Flags::Better;
			options.refineRoundsBC6H = 3;
			options.seedPoints = 3;
			break;

		case E_Quality::Ultra:
			options.flags = cvtt::Flags::Ultra;
			options.refineRoundsBC6H = 3;
			options.seedPoints = 4;
			break;
		}

		outData.resize(blockCount * sizeof(Block16));
		Block16* const packedImage = reinterpret_cast<Block16*>(outData.data());

		for (uint32 blockIdx = 0u; blockIdx < blockCount; blockIdx += cvtt::NumParallelBlocks)
		{
			uint8 const* const blockPixels = reinterpret_cast<uint8 const*>(sourceData) + (2 * blockIdx * 4u * 16u); // channel count * pixels per block
			Block16* compressedBlock = &packedImage[blockIdx];
			cvtt::Kernels::EncodeBC6HU(reinterpret_cast<uint8*>(compressedBlock), reinterpret_cast<cvtt::PixelBlockF16 const*>(blockPixels), options);
		}

		break;
	}

	case render::E_ColorFormat::BC7_RGBA:
	case render::E_ColorFormat::BC7_SRGBA:
	{
		bc7enc_compress_block_init();

		outData.resize(blockCount * sizeof(Block16));
		Block16* const packedImage = reinterpret_cast<Block16*>(outData.data());

		bc7enc_compress_block_params bc7PackParams;
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

		for (uint32 blockIdx = 0u; blockIdx < blockCount; ++blockIdx)
		{
			uint8 const* const blockPixels = reinterpret_cast<uint8 const*>(sourceData) + (blockIdx * 4u * 16u); // channel count * pixels per block
			Block16* compressedBlock = &packedImage[blockIdx];
			bc7enc_compress_block(compressedBlock, reinterpret_cast<void const*>(blockPixels), &bc7PackParams);
		}

		break;
	}

	default:
		ET_ASSERT(false, "unhandled compressed format");
		return false;
	}

	return true;
}

//-------------------------------------
// TextureCompression::CompressImageU8
//
// Specialization for raster images
//
bool TextureCompression::CompressImageU8(RasterImage const& image, 
	render::E_ColorFormat const format, 
	E_Quality const compressionQuality, 
	std::vector<uint8>& outData)
{
	// sequential read
	RasterImage::ColorU8* sourceBlocks = new RasterImage::ColorU8[image.GetWidth() * image.GetHeight()];
	image.RearrangeInBlocks(4u, 4u, sourceBlocks);

	uint32 const blockCount = (image.GetWidth() * image.GetHeight()) / 16u;

	bool const success = CompressImage(reinterpret_cast<void const*>(sourceBlocks), blockCount, format, compressionQuality, outData);

	delete[] sourceBlocks;
	return success;
}


} // namespace pl
} // namespace et

