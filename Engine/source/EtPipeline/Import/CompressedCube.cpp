#include "stdafx.h"
#include "CompressedCube.h"

#include <ConvectionKernels/ConvectionKernels.h>


namespace et {
namespace pl {


//-----------------------
// CompressedCube::c-tor
//
CompressedCube::CompressedCube(render::TextureData const& cubeMap, TextureCompression::E_Quality const quality)
{
	CompressedCube* mip = this;

	ivec2 const res = cubeMap.GetResolution();
	ET_ASSERT(res.x == res.y, "cubemaps must be square");
	ET_ASSERT(RasterImage::GetClosestPowerOf2(res.x) == static_cast<uint32>(res.x), "cubemap size must be a power of 2");

	uint32 size = static_cast<uint32>(res.x);
	for (int32 mipLevel = 0; mipLevel < cubeMap.GetNumMipLevels(); ++mipLevel)
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
void CompressedCube::CompressFromTexture(render::TextureData const& cubeMap, 
	TextureCompression::E_Quality const quality, 
	uint8 const mipLevel,
	uint32 const size)
{
	static size_t const s_BlockDim = 4u;
	static size_t const s_ChannelCount = 4u;
	static render::E_ColorFormat const compressedFormat = render::E_ColorFormat::BC6H_RGB;

	size_t const faceSize = s_ChannelCount * static_cast<size_t>(size) * static_cast<size_t>(size); // count of int16, not bytes

	uint32 const blocksX = size / s_BlockDim;
	uint32 const blocksY = size / s_BlockDim;

	size_t const blockCount = faceSize / (16u * s_ChannelCount); // per face, 4x4 = 16 pixels
	size_t const compressedFaceSize = blockCount * 16u; // BC6H block size is 16 bytes
	m_CompressedData.resize(compressedFaceSize * render::TextureData::s_NumCubeFaces);

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
	int16* const cubePixels = new int16[faceSize * render::TextureData::s_NumCubeFaces];
	render::I_GraphicsContextApi* const api = render::ContextHolder::GetRenderContext();
	api->GetTextureData(cubeMap, mipLevel, render::E_ColorFormat::RGBA, render::E_DataType::Half, reinterpret_cast<void*>(cubePixels));

	for (uint8 faceIdx = 0u; faceIdx < render::TextureData::s_NumCubeFaces; ++faceIdx)
	{
		int16* const facePixels = cubePixels + (faceSize * static_cast<size_t>(faceIdx));

		int16* const faceBlockPixels = new int16[paddedBlockCount * (16u * s_ChannelCount)];

		// swizzle into blocks
		{
			uint32 writeAddress = 0u;
			for (uint32 blockY = 0u; blockY < blocksY; ++blockY)
			{
				uint32 const yOffset = blockY * s_BlockDim;
				for (uint32 blockX = 0u; blockX < blocksX; ++blockX)
				{
					uint32 const x = blockX * s_BlockDim;
					for (uint32 blockRow = 0u; blockRow < s_BlockDim; blockRow++)
					{
						uint32 const readAddress = x + (size * (yOffset + blockRow));
						memcpy(reinterpret_cast<void*>(&faceBlockPixels[writeAddress * s_ChannelCount]),
							reinterpret_cast<void const*>(&facePixels[readAddress * s_ChannelCount]),
							s_BlockDim * sizeof(int16) * s_ChannelCount);

						writeAddress += s_BlockDim;
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
			size_t const toRemove = padding * static_cast<size_t>(render::TextureFormat::GetBlockByteCount(compressedFormat));
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

