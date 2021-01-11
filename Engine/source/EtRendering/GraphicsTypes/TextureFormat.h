#pragma once



namespace et {
namespace render {


//------------------------------------------
class TextureFormatRegistrationLinkEnforcer
{
	DECLARE_FORCED_LINKING()
};

//-----------------------
// E_CompressionSetting
//
// Combined with channel count will ultimately define the GPU storage format and the disc storage format
//
enum class E_CompressionSetting : uint8
{
	Invalid = 0,

	Default, // BC1 or BC3 depending on alpha channel - sRGB option, but masks shouldn't use it
	NormalMap, // BC5
	GrayScale, // R8 or sRGB
	DisplacementMap, // R8 / R16
	VectorDisplacementMap, // RGB7
	HDR, // RGB16F
	UI, // RGBA8 / sRGBA8
	Alpha, // BC4
	SdfFont, // RGBA8 for channel usage - could make single channel in future
	// CompressedHDR, // BC6H - not supported currently
	BC7 // High quality version of default when targeting modern GPUs
};

bool RequiresCompression(E_CompressionSetting const setting);

//---------------
// E_SrgbSetting
//
enum class E_SrgbSetting : uint8
{
	None,
	//OnCook,
	OnLoad
};

//-------------
// TextureFile
//
// Custom file format for storing compressed textures
//
class TextureFile
{
public:
	static std::string const s_Header; // "ETTEX"

private:
	std::string m_WriteVersion;
	E_TextureType m_Type;
	ivec2 m_Resolution;
	E_ColorFormat m_ColorLayout;
	E_CompressionSetting m_CompressionSetting;
	E_SrgbSetting m_Srgb;
	uint8 m_MipCount;
};


} // namespace render
} // namespace et
