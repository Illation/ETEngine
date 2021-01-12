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


//---------------------------------
// EditableTextureAsset
//
class EditableTextureAsset final : public EditorAsset<render::TextureData>
{
	DECLARE_FORCED_LINKING()
	RTTR_ENABLE(EditorAsset<render::TextureData>)
	REGISTRATION_FRIEND_NS(pl)
public:
	enum class E_CompressionQuality : uint8
	{
		Low,
		Medium,
		High,
		Ultra
	};

	// Construct destruct
	//---------------------
	EditableTextureAsset() : EditorAsset<render::TextureData>() {}
	virtual ~EditableTextureAsset() = default;

	// interface
	//-----------
protected:
	bool LoadFromMemory(std::vector<uint8> const& data) override;

	// utility
	//---------
private:
	bool LoadImage(RasterImage& image, std::vector<uint8> const& data) const;
	uint32 GetPow2Size(uint32 const width, uint32 const height, bool adjustByGraphicsSettings) const;
	render::E_ColorFormat GetOutputFormat(render::E_CompressionSetting const setting, bool const supportAlpha, bool const useSrgb) const;
	uint8 GetInputChannelCount(render::E_ColorFormat const format) const;


	// Data
	///////

	render::E_SrgbSetting m_Srgb = render::E_SrgbSetting::None;

	bool m_SupportAlpha = true;
	uint16 m_MaxSize = 0u;

	render::E_CompressionSetting m_CompressionSetting = render::E_CompressionSetting::Default;
	E_CompressionQuality m_CompressionQuality = E_CompressionQuality::Medium;
};


} // namespace pl
} // namespace et
