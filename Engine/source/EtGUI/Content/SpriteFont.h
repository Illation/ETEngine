#pragma once
#include <EtCore/Content/AssetPointer.h>

#include <EtRendering/GraphicsTypes/TextureData.h>


namespace et { namespace pl {
	class EditableFontAsset;
} }


namespace et {
namespace gui {


//---------------------------------
// FontMetric
//
// Information about positioning of individual characters in a font
//
struct FontMetric
{
	// accessors
	//-----------
	vec2 GetKerningVec(wchar_t previous) const;

	// Data
	///////

	bool IsValid = false;
	wchar_t Character = 0;

	// dimensions
	uint16 Width = 0;
	uint16 Height = 0;
	int16 OffsetX = 0;
	int16 OffsetY = 0;

	// spacing between characters
	float AdvanceX = 0;
	std::map<wchar_t, vec2> Kerning;

	// addressing in texture
	uint8 Page = 0;
	uint8 Channel = 0;
	vec2 TexCoord = 0;
};

//---------------------------------
// SpriteFont
//
// Font renderable in realtime
//
class SpriteFont final
{
	// definitions
	//-------------
private:
	friend class TextRenderer;
	friend class FontLoader;
	friend class FontAsset;
	friend class pl::EditableFontAsset;

public:
	static bool IsCharValid(const wchar_t& character);

	static int32 const s_MinCharId = 0;
	static int32 const s_MaxCharId = 255;
	static int32 const s_CharCount = s_MaxCharId - s_MinCharId + 1;

	// construct destruct
	//--------------------
	SpriteFont() = default;
	~SpriteFont() = default;

	SpriteFont(SpriteFont const& other);
	SpriteFont& operator=(SpriteFont const& other);

	// accessors
	//-----------j
	render::TextureData const* GetAtlas() const { return (m_TextureAsset != nullptr) ? m_TextureAsset.get() : m_Texture.Get(); }
	int16 GetFontSize() const { return m_FontSize; }
	FontMetric const& GetMetric(wchar_t const& character) const;

	// utility
	//---------
private:
	FontMetric& GetMetric(wchar_t const& character);
	void SetMetric(FontMetric const& metric, wchar_t const& character);

	// Data
	///////

	// font info
	std::string m_FontName;
	int16 m_FontSize = 0;

	// character info
	FontMetric m_CharTable[s_CharCount];
	int32 m_CharacterCount = 0;
	int32 m_CharacterSpacing = 1;
	bool m_UseKerning = false;

	// sprite info
	UniquePtr<render::TextureData const> m_Texture; // editor
	AssetPtr<render::TextureData> m_TextureAsset; // runtime
};

//---------------------------------
// FontAsset
//
// Loadable Font Data
//
class FontAsset final : public core::Asset<SpriteFont, false>
{
	// definitions
	//-------------
	RTTR_ENABLE(core::Asset<SpriteFont, false>)
	DECLARE_FORCED_LINKING()
public:
	static float const s_KerningAdjustment;

	// Construct destruct
	//---------------------
	FontAsset() : core::Asset<SpriteFont, false>() {}
	virtual ~FontAsset() = default;

	// Asset overrides
	//---------------------
	bool LoadFromMemory(std::vector<uint8> const& data) override;

private:
	SpriteFont* LoadFnt(std::vector<uint8> const& binaryContent);
};


} // namespace gui
} // namespace et
