#pragma once
#include <EtRendering/GraphicsTypes/TextureData.h>

#include <EtPipeline/Content/EditorAsset.h>
#include <EtPipeline/Import/TextureCompression.h>


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
	static bool LoadImage(RasterImage& image, std::vector<uint8> const& data);

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


	// Data
	///////

private:
	render::TextureFormat::E_Srgb m_Srgb = render::TextureFormat::E_Srgb::None;

	bool m_SupportAlpha = true;
	uint16 m_MaxSize = 0u;

	bool m_UseMipMaps = true;

	TextureCompression::E_Setting m_CompressionSetting = TextureCompression::E_Setting::Default;
	TextureCompression::E_Quality m_CompressionQuality = TextureCompression::E_Quality::Medium;
};


} // namespace pl
} // namespace et
