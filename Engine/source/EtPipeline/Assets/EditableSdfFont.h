#pragma once
#include <EtGUI/Content/SdfFont.h>

#include <EtPipeline/Content/EditorAsset.h>


namespace et {
	REGISTRATION_NS(pl);
}


namespace et {
namespace pl {


//---------------------------------
// EditableSdfFontAsset
//
class EditableSdfFontAsset final : public EditorAsset<gui::SdfFont>
{
	RTTR_ENABLE(EditorAsset<gui::SdfFont>)
	REGISTRATION_FRIEND_NS(pl)
	DECLARE_FORCED_LINKING()

	static std::string const s_FontFileExt;
public:

	//---------------------------------
	// Charset
	//
	struct Charset
	{
		char32 m_Start = 0u;
		char32 m_End = 0u; // inclusive
	};

	// Construct destruct
	//---------------------
	EditableSdfFontAsset() : EditorAsset<gui::SdfFont>() {}
	virtual ~EditableSdfFontAsset() = default;

	// interface
	//-----------
protected:
	bool LoadFromMemory(std::vector<uint8> const& data) override;

	void SetupRuntimeAssetsInternal() override;
	bool GenerateInternal(BuildConfiguration const& buildConfig, std::string const& dbPath) override;

	bool GenerateRequiresLoadData() const override { return true; }

	// utility
	//---------
private:
	gui::SdfFont* LoadTtf(std::vector<uint8> const& binaryContent);

	void PopulateTextureParams(render::TextureParameters& params) const;

	bool GenerateBinFontData(std::vector<uint8>& data, gui::SdfFont const* const font, std::string const& atlasName);
	bool GenerateTextureData(std::vector<uint8>& data, render::TextureData const* const texture);

	// Data
	///////
	uint32 m_FontSize = 42u;
	uint32 m_Padding = 1u;
	uint32 m_Spread = 5u;
	uint32 m_HighRes = 32u;
	float m_EmPer100Weight = 0.01f;
	std::vector<Charset> m_CharSets;
};


} // namespace pl
} // namespace et
