#pragma once


namespace et {
namespace pl {


//-------------
// RasterImage
//
// 4 channel bitmap
// 8 bits precision - could template in the future to allow for 16 bit precision
// Utility class to help with block based texture compression
//
class RasterImage final
{
	// definitions
	//-------------
public:
	static uint32 const s_NumChannels;

	//---------------------------------------------
	// Helper struct for an 8 bit color / pixel
	struct ColorU8
	{
		ColorU8() : ColorU8(0u, 0u, 0u, 0u) {}
		ColorU8(uint8 const red, uint8 const green, uint8 const blue, uint8 const alpha);

		inline uint8& operator[] (uint8 const i);
		inline uint8 operator[] (uint8 const i) const;

		uint8 m_Channels[4u];
	};

	static uint32 GetClosestPowerOf2(uint32 const size);

	// construct destruct
	//--------------------
	RasterImage() = default;
	RasterImage(uint32 const width, uint32 const height);
	~RasterImage() = default;

	// functionality
	//---------------
	inline void AllocatePixels();
	inline void SetSize(uint32 const width, uint32 const height);
	void SetPixels(ColorU8 const* const pixels);
	inline void SetPixel(uint32 const x, uint32 const y, ColorU8 const pixel);
	void Resize(uint32 const width, uint32 const height);
	void Swizzle(uint8 const r, uint8 const g, uint8 const b, uint8 const a);
	void GenerateMipChain(uint32 const smallestSize);

	// acccessors
	//------------
	uint32 GetWidth() const { return m_Width; }
	uint32 GetHeight() const { return m_Height; }
	inline uint32 GetByteCount() const;

	inline ColorU8 const* GetPixels() const;
	inline ColorU8* GetPixels();
	std::vector<uint8> GetPixels(uint8 const numChannels) const;

	inline ColorU8 const& At(uint32 const x, uint32 const y) const;
	inline void GetBlock(uint32 const blockX, uint32 const blockY, uint32 const blockWidth, uint32 const blockHeight, ColorU8* const outBlock) const;
	void RearrangeInBlocks(uint32 const blockWidth, uint32 const blockHeight, ColorU8* const outBlocks) const;

	RasterImage const* GetMipChild() const { return m_MipChild.Get(); }
	uint8 GetMipLevelCount() const;


	// Data
	///////

private:
	uint32 m_Width = 0; // 32 bit should be enought to access any pixel even in large tex
	uint32 m_Height = 0;

	std::vector<ColorU8> m_Pixels;

	UniquePtr<RasterImage> m_MipChild;
};


} // namespace pl
} // namespace et


#include "RasterImage.inl"
