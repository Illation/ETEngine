#pragma once
#include <EtRendering/GraphicsTypes/TextureData.h>
#include <EtRendering/GraphicsTypes/TextureFormat.h>

#include <EtPipeline/Content/EditorAsset.h>


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
