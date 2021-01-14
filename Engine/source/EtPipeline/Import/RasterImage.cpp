#include "stdafx.h"
#include "RasterImage.h"

#include <stb/stb_image_resize.h>


namespace et {
namespace pl {


//==========================
// Raster Image :: Color U8
//==========================


//----------------
// ColorU8::c-tor
//
RasterImage::ColorU8::ColorU8(uint8 const red, uint8 const green, uint8 const blue, uint8 const alpha)
{
	m_Channels[0] = red;
	m_Channels[1] = green;
	m_Channels[2] = blue;
	m_Channels[3] = alpha;
}


//==============
// Raster Image
//==============


// static
uint32 const RasterImage::s_NumChannels = 4;

//---------------------------------
// RasterImage::GetClosestPowerOf2
//
// rounds up
//
uint32 RasterImage::GetClosestPowerOf2(uint32 const size)
{
	uint32 power = 1u;
	while (power < size)
	{
		power *= 2u;
	}

	return power;
}


//--------------------
// RasterImage::c-tor
//
// Init as black / translucent
//
RasterImage::RasterImage(uint32 const width, uint32 const height)
	: m_Width(width)
	, m_Height(height)
{
	AllocatePixels();
}

//--------------------
// RasterImage::d-tor
//
RasterImage::~RasterImage()
{
	delete m_MipChild;
}

//------------------------
// RasterImage::SetPixels
//
void RasterImage::SetPixels(ColorU8 const* const pixels)
{
	memcpy(&m_Pixels[0], &pixels[0], m_Width * m_Height * sizeof(ColorU8));
}

//---------------------
// RasterImage::Resize
//
// We might need to support different filters, handle sRGB differently etc
//
void RasterImage::Resize(uint32 const width, uint32 const height)
{
	uint8* outPixels = new uint8[width * height * s_NumChannels];
	stbir_resize_uint8(reinterpret_cast<uint8 const*>(m_Pixels.data()), 
		m_Width, 
		m_Height, 
		0, 
		outPixels, 
		width, 
		height, 
		0, 
		static_cast<int32>(s_NumChannels));

	m_Width = width;
	m_Height = height;
	AllocatePixels();
	SetPixels(reinterpret_cast<ColorU8 const*>(outPixels));
}

//----------------------
// RasterImage::Swizzle
//
// Set the contents of our channels to the channel contents at the given indices
//
void RasterImage::Swizzle(uint8 const r, uint8 const g, uint8 const b, uint8 const a)
{
	ET_ASSERT((r | g | b | a) <= 3);
	for (ColorU8& pixel : m_Pixels)
	{
		ColorU8 const tmp(pixel);
		pixel[0] = tmp[r];
		pixel[1] = tmp[g];
		pixel[2] = tmp[b];
		pixel[3] = tmp[a];
	}
}

//-------------------------------
// RasterImage::GenerateMipChain
//
// Create mip maps until we reach a certain level
//  we don't use the resize function here, as we don't need fancy filters and don't want to copy loads of memory around
//
void RasterImage::GenerateMipChain()
{
	ET_ASSERT(m_MipChild == nullptr, "mip chain already exists");
	ET_ASSERT(m_Width == m_Height, "only square textures are supported for mip maps");
	ET_ASSERT(GetClosestPowerOf2(m_Width) == m_Width, "textures should be a power of 2");

	if (m_Width <= 1)
	{
		return;
	}

	uint32 const size = m_Width / 2u;
	m_MipChild = new RasterImage(size, size);
	m_MipChild->AllocatePixels();

	for (uint32 y = 0u; y < size; ++y)
	{
		for (uint32 x = 0u; x < size; ++x)
		{
			// access block of 4 pixels
			ColorU8 pixels[4u];
			GetBlock(x, y, 2u, 2u, pixels);

			// set average
			m_MipChild->SetPixel(x, y, 
				ColorU8(
					static_cast<uint8>((static_cast<uint16>(pixels[0][0]) + static_cast<uint16>(pixels[1][0]) + static_cast<uint16>(pixels[2][0]) + static_cast<uint16>(pixels[3][0])) >> 2u), 
					static_cast<uint8>((static_cast<uint16>(pixels[0][1]) + static_cast<uint16>(pixels[1][1]) + static_cast<uint16>(pixels[2][1]) + static_cast<uint16>(pixels[3][1])) >> 2u),
					static_cast<uint8>((static_cast<uint16>(pixels[0][2]) + static_cast<uint16>(pixels[1][2]) + static_cast<uint16>(pixels[2][2]) + static_cast<uint16>(pixels[3][2])) >> 2u),
					static_cast<uint8>((static_cast<uint16>(pixels[0][3]) + static_cast<uint16>(pixels[1][3]) + static_cast<uint16>(pixels[2][3]) + static_cast<uint16>(pixels[3][3])) >> 2u)
				));
		}
	}

	m_MipChild->GenerateMipChain();
}

//------------------------
// RasterImage::GetPixels
//
// Return a bitmap with a subset of the required channels
//
std::vector<uint8> RasterImage::GetPixels(uint8 const numChannels) const
{
	ET_ASSERT((numChannels > 0u) && (numChannels < 4u));

	std::vector<uint8> outPixels;
	outPixels.reserve(m_Width * m_Height * numChannels);

	for (ColorU8 const pixel : m_Pixels)
	{
		for (uint8 chan = 0u; chan < numChannels; ++chan)
		{
			outPixels.push_back(pixel[chan]);
		}
	}

	return outPixels;
}

//-------------------------------
// RasterImage::GetMipLevelCount
//
uint8 RasterImage::GetMipLevelCount() const
{
	uint8 ret = 0u;

	RasterImage const* mipImage = m_MipChild;
	while (mipImage != nullptr)
	{
		++ret;
		mipImage = mipImage->GetMipChild();
	}

	return ret;
}


} // namespace pl
} // namespace et
