#pragma once
#include "RasterImage.h"

#include <EtRendering/GraphicsTypes/TextureFormat.h>


namespace et {
namespace pl {


//------------------------------
// TextureCompression
//
// Utility class for compression functionality
//
class TextureCompression final
{
	typedef void(*T_GetBlock4x4Fn)(void const* const, uint32 const, uint32 const, void* const); // imageData, blockX, blockY, buffer
	static size_t const s_PixelBufferSize;

public:

	//-----------------------------------------
	enum class E_Quality : uint8
	{
		Low,
		Medium,
		High,
		Ultra
	};

	//-----------------------
	// E_CompressionSetting
	//
	// Combined with channel count will ultimately define the GPU storage format and the disc storage format
	//
	enum class E_Setting : uint8
	{
		Invalid = 0,

		Default, // BC1 or BC3 depending on alpha channel - sRGB option, but masks shouldn't use it
		NormalMap, // BC5
		GrayScale, // R8 or sRGB
		DisplacementMap, // R8 / R16
		VectorDisplacementMap, // RGB8
		HDR, // RGB16F
		UI, // RGBA8 / sRGBA8
		Alpha, // BC4
		SdfFont, // RGBA8 for channel usage - could make single channel in future
		CompressedHDR, // BC6H - not supported currently
		BC7 // High quality version of default when targeting modern GPUs
	};

	// write to file
	static bool WriteTextureFile(std::vector<uint8>& outFileData,
		RasterImage& source,
		E_Setting const compressionSetting,
		E_Quality const compressionQuality,
		bool const supportsAlpha,
		render::TextureFormat::E_Srgb const srgb,
		uint16 const maxSize,
		bool const forceResolution,
		bool const useMipMaps);

	// utility
	static uint32 GetPow2Size(uint32 const width, uint32 const height, uint16 const maxSize, bool adjustByGraphicsSettings);
	static render::E_ColorFormat GetOutputFormat(E_Setting const setting, bool const supportAlpha, bool const useSrgb);

	// Generic compressor
	static bool CompressImage(void const* const sourceData, 
		uint32 const blockCount, 
		render::E_ColorFormat const format,
		E_Quality const compressionQuality,
		std::vector<uint8>& outData);

	// For U8 source data
	static bool CompressImageU8(RasterImage const& image,
		render::E_ColorFormat const format,
		E_Quality const compressionQuality,
		std::vector<uint8>& outData);
};


} // namespace pl
} // namespace et

