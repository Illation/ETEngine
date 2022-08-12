#pragma once


// Inline Functions
/////////////////////


namespace et {
namespace pl {


//==========================
// Raster Image :: Color U8
//==========================


//-----------------------
// ColorU8::operator []
//
uint8& RasterImage::ColorU8::operator[](uint8 const i)
{
	ET_ASSERT_PARANOID(i < 4u);
	return m_Channels[i];
}

//-----------------------
// ColorU8::operator []
//
uint8 RasterImage::ColorU8::operator[](uint8 const i) const
{
	ET_ASSERT_PARANOID(i < 4u);
	return m_Channels[i];
}


//==============
// Raster Image 
//==============


//-----------------------------
// RasterImage::AllocatePixels
//
// Create space according to the image size - overwrites existing data
//
void RasterImage::AllocatePixels()
{
	m_Pixels.resize(static_cast<size_t>(m_Width) * static_cast<size_t>(m_Height));
}

//----------------------
// RasterImage::SetSize
//
// Make sure to reallocate pixels after setting size
//
void RasterImage::SetSize(uint32 const width, uint32 const height)
{
	ET_ASSERT(m_MipChild == nullptr, "setting size while there is already a generated mip chain is not supported");

	m_Width = width;
	m_Height = height;
}

//---------------------------
// RasterImage::GetByteCount
//
uint32 RasterImage::GetByteCount() const
{
	ET_ASSERT(sizeof(ColorU8) == 4u);
	return m_Width * m_Height * sizeof(ColorU8);
}

//------------------------
// RasterImage::GetPixels
//
RasterImage::ColorU8 const* RasterImage::GetPixels() const
{
	return m_Pixels.data();
}

//------------------------
// RasterImage::GetPixels
//
RasterImage::ColorU8* RasterImage::GetPixels() 
{
	return m_Pixels.data();
}

//-----------------------
// RasterImage::SetPixel
//
void RasterImage::SetPixel(uint32 const x, uint32 const y, ColorU8 const pixel)
{
	ET_ASSERT_PARANOID((x < m_Width) && (y < m_Height));
	m_Pixels[x + (m_Width * y)] = pixel;
}

//-----------------
// RasterImage::At
//
// Return as a reference so we can use it as an array
//
RasterImage::ColorU8 const& RasterImage::At(uint32 const x, uint32 const y) const
{
	ET_ASSERT_PARANOID((x < m_Width) && (y < m_Height));
	return m_Pixels[x + (m_Width * y)];
}

//-----------------------
// RasterImage::GetBlock
//
void RasterImage::GetBlock(uint32 const blockX, uint32 const blockY, uint32 const blockWidth, uint32 const blockHeight, ColorU8* const outBlock) const
{
	ET_ASSERT_PARANOID((blockX * blockWidth + blockWidth) <= m_Width);
	ET_ASSERT_PARANOID((blockY * blockHeight + blockHeight) <= m_Height);

	for (uint32 y = 0u; y < blockHeight; y++)
	{
		memcpy(reinterpret_cast<void*>(outBlock + (y * blockWidth)),
			reinterpret_cast<void const*>(&At(blockX * blockWidth, (blockY * blockHeight) + y)),
			blockWidth * sizeof(ColorU8));
	}
}


} // namespace pl
} // namespace et
