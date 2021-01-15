#pragma once
#include <EtRendering/GraphicsTypes/TextureData.h>
#include <EtRendering/GraphicsTypes/TextureFormat.h>

#include <EtPipeline/Content/EditorAsset.h>
#include <EtPipeline/Import/RasterImage.h>


namespace et {
	REGISTRATION_NS(pl);
}


namespace et {
namespace pl {


//---------------------------------
// EditableTextureAsset
//
class EditableTextureAsset final : public EditorAsset<render::TextureData>
{
	DECLARE_FORCED_LINKING()
	RTTR_ENABLE(EditorAsset<render::TextureData>)
	REGISTRATION_FRIEND_NS(pl)

	//definitions
	//------------
public:

	//-----------------------------------------
	enum class E_CompressionQuality : uint8
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
	enum class E_CompressionSetting : uint8
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
		// CompressedHDR, // BC6H - not supported currently
		BC7 // High quality version of default when targeting modern GPUs
	};

	static bool LoadImage(RasterImage& image, std::vector<uint8> const& data);
	static bool WriteTextureFile(std::vector<uint8>& outFileData,
		RasterImage& source,
		E_CompressionSetting const compressionSetting,
		E_CompressionQuality const compressionQuality,
		bool const supportsAlpha,
		render::TextureFile::E_Srgb const srgb,
		uint16 const maxSize,
		bool const forceResolution, 
		bool const useMipMaps);


	// Construct destruct
	//---------------------
	EditableTextureAsset() : EditorAsset<render::TextureData>() {}
	virtual ~EditableTextureAsset() = default;

	// interface
	//-----------
protected:
	bool LoadFromMemory(std::vector<uint8> const& data) override;

	bool GenerateInternal(BuildConfiguration const& buildConfig, std::string const& dbPath) override;
	bool GenerateRequiresLoadData() const override { return true; }

	// utility
	//---------
private:
	static uint32 GetPow2Size(uint32 const width, uint32 const height, uint16 const maxSize, bool adjustByGraphicsSettings);
	static render::E_ColorFormat GetOutputFormat(E_CompressionSetting const setting, bool const supportAlpha, bool const useSrgb);
	static uint8 GetInputChannelCount(render::E_ColorFormat const format);
	static bool CompressImage(RasterImage const& image, 
		render::E_ColorFormat const format, 
		E_CompressionQuality const compressionQuality, 
		std::vector<uint8>& outData);


	// Data
	///////

	render::TextureFile::E_Srgb m_Srgb = render::TextureFile::E_Srgb::None;

	bool m_SupportAlpha = true;
	uint16 m_MaxSize = 0u;

	bool m_UseMipMaps = true;

	E_CompressionSetting m_CompressionSetting = E_CompressionSetting::Default;
	E_CompressionQuality m_CompressionQuality = E_CompressionQuality::Medium;
};


} // namespace pl
} // namespace et
