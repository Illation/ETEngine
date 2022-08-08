#include "stdafx.h"
#include "CompressedCube.h"

#include <ConvectionKernels/ConvectionKernels.h>


namespace et {
namespace pl {


//-----------------------
// CompressedCube::c-tor
//
CompressedCube::CompressedCube(rhi::TextureData const& cubeMap, TextureCompression::E_Quality const quality)
{
	CompressedCube* mip = this;

	ivec2 const res = cubeMap.GetResolution();
	ET_ASSERT(res.x == res.y, "cubemaps must be square");
	ET_ASSERT(RasterImage::GetClosestPowerOf2(res.x) == static_cast<uint32>(res.x), "cubemap size must be a power of 2");

	uint32 size = static_cast<uint32>(res.x);
	for (int32 mipLevel = 0; mipLevel < cubeMap.GetNumMipLevels() + 1; ++mipLevel)
	{
		mip->CompressFromTexture(cubeMap, quality, static_cast<uint8>(mipLevel), size);

		size /= 2u;
		if (size < 4u)
		{
			return;
		}

		if (mipLevel < cubeMap.GetNumMipLevels() - 1)
		{
			mip->CreateMip();
			mip = mip->GetChildMip();
		}
	}
}

//-----------------------
// CompressedCube::d-tor
//
CompressedCube::~CompressedCube()
{
	delete m_ChildMip;
}

//-------------------------------------
// CompressedCube::CompressFromTexture
//
void CompressedCube::CompressFromTexture(rhi::TextureData const& cubeMap, 
	TextureCompression::E_Quality const quality, 
	uint8 const mipLevel,
	uint32 const size)
{
	static size_t const s_BlockDim = 4u;
	static size_t const s_ChannelCount = 4u;
	static size_t const s_UncompressedPixelSize = s_ChannelCount * 2u;
	static size_t const s_UncompressedBlockRow = s_UncompressedPixelSize * s_BlockDim;
	static size_t const s_UncompressedBlockSize = s_UncompressedBlockRow * s_BlockDim;
	static rhi::E_ColorFormat const compressedFormat = rhi::E_ColorFormat::BC6H_RGB;

	size_t const faceSize = s_UncompressedPixelSize * static_cast<size_t>(size) * static_cast<size_t>(size); // count of int16, not bytes
	size_t const rowLength = static_cast<size_t>(size) * s_UncompressedPixelSize;

	uint32 const blocksX = size / s_BlockDim;
	uint32 const blocksY = size / s_BlockDim;

	size_t const blockCount = faceSize / s_UncompressedBlockSize; // per face, 4x4 = 16 pixels
	size_t const compressedFaceSize = blockCount * 16u; // BC6H block size is 16 bytes
	m_CompressedData.resize(compressedFaceSize * rhi::TextureData::s_NumCubeFaces);

	// BC6H texture compression with Convection kernels requires to process this amount of pixels at a time, so we need to provide it
	size_t padding = 0;
	{
		size_t const excess = blockCount % cvtt::NumParallelBlocks;
		if (excess != 0u)
		{
			padding = cvtt::NumParallelBlocks - excess;
		}
	}

	size_t const paddedBlockCount = blockCount + padding;

	// read pixels from GPU
	uint8* const cubePixels = new uint8[faceSize * rhi::TextureData::s_NumCubeFaces];
	rhi::I_GraphicsContextApi* const api = rhi::ContextHolder::GetRenderContext();
	api->GetTextureData(cubeMap, mipLevel, rhi::E_ColorFormat::RGBA, rhi::E_DataType::Half, reinterpret_cast<void*>(cubePixels));

	for (uint8 faceIdx = 0u; faceIdx < rhi::TextureData::s_NumCubeFaces; ++faceIdx)
	{
		uint8 const* const facePixels = cubePixels + (faceSize * static_cast<size_t>(faceIdx));

		uint8* const faceBlockPixels = new uint8[faceSize];

		// swizzle into blocks
		{
			uint32 writeAddress = 0u;
			for (uint32 blockY = 0u; blockY < blocksY; ++blockY)
			{
				uint32 const yOffset = blockY * s_BlockDim;
				for (uint32 blockX = 0u; blockX < blocksX; ++blockX)
				{
					uint32 const x = blockX * s_UncompressedBlockRow;
					for (uint32 blockRow = 0u; blockRow < s_BlockDim; blockRow++)
					{
						uint32 const readAddress = x + (rowLength * (yOffset + blockRow));
						memcpy(reinterpret_cast<void*>(faceBlockPixels + writeAddress),
							reinterpret_cast<void const*>(facePixels + readAddress),
							s_UncompressedBlockRow);

						writeAddress += s_UncompressedBlockRow;
					}
				}
			}
		}

		std::vector<uint8> faceData;
		TextureCompression::CompressImage(faceBlockPixels,
			static_cast<uint32>(paddedBlockCount),
			compressedFormat,
			quality,
			faceData);

		delete[] faceBlockPixels;

		// we can now remove the padding again
		if (padding != 0)
		{
			size_t const toRemove = padding * static_cast<size_t>(rhi::TextureFormat::GetBlockByteCount(compressedFormat));
			faceData.resize(faceData.size() - toRemove);
		}

		ET_ASSERT(faceData.size() == compressedFaceSize);
		memcpy(m_CompressedData.data() + (static_cast<size_t>(faceIdx) * compressedFaceSize), faceData.data(), compressedFaceSize);
	}

	delete[] cubePixels;
}

//--------------------------
// CompressedCube::GetFace
//
void CompressedCube::CreateMip()
{
	m_ChildMip = new CompressedCube();
}


} // namespace pl
} // namespace et

