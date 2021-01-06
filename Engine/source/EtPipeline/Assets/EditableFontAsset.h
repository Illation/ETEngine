#pragma once
#include <EtRendering/GraphicsTypes/SpriteFont.h>

#include <EtPipeline/Content/EditorAsset.h>


namespace et {
	REGISTRATION_NS(pl);
}


namespace et {
namespace pl {


//---------------------------------
// EditableFontAsset
//
class EditableFontAsset final : public EditorAsset<render::SpriteFont>
{
	RTTR_ENABLE(EditorAsset<render::SpriteFont>)
	REGISTRATION_FRIEND_NS(pl)
	DECLARE_FORCED_LINKING()

	static std::string const s_FontFileExt;
	static std::string const s_TextureFileExt;
public:
	// Construct destruct
	//---------------------
	EditableFontAsset() : EditorAsset<render::SpriteFont>() {}
	virtual ~EditableFontAsset() = default;

	// interface
	//-----------
protected:
	bool LoadFromMemory(std::vector<uint8> const& data) override;

	void SetupRuntimeAssetsInternal() override;
	bool GenerateInternal(BuildConfiguration const& buildConfig) override;

	bool GenerateRequiresLoadData() const override { return true; }

	// utility
	//---------
private:
	render::SpriteFont* LoadTtf(std::vector<uint8> const& binaryContent);

	void PopulateTextureParams(render::TextureParameters& params) const;

	bool GenerateBinFontData(std::vector<uint8>& data, render::SpriteFont const* const font, std::string const& atlasName);
	bool GenerateTextureData(std::vector<uint8>& data, render::TextureData const* const texture);

	// Data
	///////
	uint32 m_FontSize = 42u;
	uint32 m_Padding = 1u;
	uint32 m_Spread = 5u;
	uint32 m_HighRes = 32u;
};


} // namespace pl
} // namespace et
