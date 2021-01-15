#pragma once


namespace et {
namespace render {


//-------------
// TextureFile
//
// Custom file format for storing compressed textures
//  - for now we will assume that textures are always encoded in the same format as the GPU expects the data to be when uploading
//  - in the future we might want to store uncompressed textures in formats like jpg/png/tga
//
struct TextureFile
{
	DECLARE_FORCED_LINKING()

public:
	//---------------
	// E_SrgbSetting
	//
	enum class E_Srgb: uint8
	{
		None,
		//OnCook,
		OnLoad
	};

	static std::string const s_Header; 
	static size_t const s_BlockPixelCount; // 16

	static bool IsCompressedFormat(E_ColorFormat const format);
	static uint8 GetBlockByteCount(render::E_ColorFormat const format);
	static size_t GetCompressedSize(uint32 const width, uint32 const height, E_ColorFormat const storageFormat);

	// File layout
	/*******************************
	char[5] header - "ETTEX"
	char const* writerVersion - null terminated - engine version when file was written
	E_TextureType type
	uint16 width
	uint16 height
	uint16 layers - for 3D textures, 1 otherwise
	uint8 mipCount - doesn't include level 0
	E_ColorFormat storageFormat - (on GPU) - this also tells us if the texture is srgb
	E_DataType dataType - invalid for compressed storage, otherwise typically UByte
	E_ColorFormat colorLayout - invalid for compressed storage - otherwise channel count and order
	mip level 0 bytes
	mip level 1 bytes
	...
	mip level[m_MipCount] bytes
	*/
};


} // namespace render
} // namespace et
