#pragma once
#include <EtRendering/GraphicsTypes/TextureData.h>
#include <EtRendering/GraphicsTypes/TextureFormat.h>

#include <EtPipeline/Content/EditorAsset.h>


namespace et {
	REGISTRATION_NS(pl);
}


namespace et {
namespace pl {


typedef uint64 T_Block8;
struct Block16
{
	uint64 m_Val[2];
};


//---------
// ColorU8
//
// Helper struct for an 8 bit color / pixel
//
struct ColorU8
{
	ColorU8(uint8 const red, uint8 const green, uint8 const blue, uint8 const alpha)
	{
		m_Channels[0] = red;
		m_Channels[1] = green;
		m_Channels[2] = blue;
		m_Channels[3] = alpha;
	}

	uint8 m_Channels[4];
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
	int32 GetPow2Size(int32 const width, int32 const height, bool adjustByGraphicsSettings) const;
	render::E_ColorFormat GetOutputFormat(render::E_CompressionSetting const setting, bool const supportAlpha, bool const useSrgb) const;
	uint8 GetInputChannelCount(render::E_ColorFormat const format) const;
	void GetBlock4x4(uint8 const* const pixels, int32 const width, int32 const height, uint16 const blockX, uint16 const blockY, ColorU8* const outBlock);


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
