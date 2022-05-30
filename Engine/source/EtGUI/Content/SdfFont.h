#pragma once
#include <EtCore/Content/AssetPointer.h>

#include <EtRendering/GraphicsTypes/TextureData.h>


// fwd
namespace et { namespace pl {
	class EditableSdfFontAsset;
} }


namespace et {
namespace gui {
	

//---------------------------------
// SdfFont
//
// Signed distance field font data
//
class SdfFont final
{
	// definitions
	//-------------
private:
	friend class SdfFontAsset;
	friend class pl::EditableSdfFontAsset;

public:

	//---------------------------------
	// E_Flags
	//
	// General info about the font face
	//
	typedef uint8 T_Flags;
	enum E_Flags : T_Flags
	{
		F_None = 0,

		F_Smooth		= 1 << 0, // unused
		F_Unicode		= 1 << 1, // unused
		F_Italic		= 1 << 2,
		F_Bold			= 1 << 3,
		F_FixedHeight	= 1 << 4, // unused

		F_All = 0xFF
	};

	//---------------------------------
	// Metric
	//
	// Information about positioning of individual characters in a font
	//
	struct Metric
	{
		// accessors
		//-----------
		vec2 GetKerningVec(char32 const previous) const;

		// Data
		///////

		bool m_IsValid = false;
		char32 m_Character = 0;

		// dimensions
		uint16 m_Width = 0;
		uint16 m_Height = 0;
		int16 m_OffsetX = 0;
		int16 m_OffsetY = 0;

		// spacing between characters
		float m_AdvanceX = 0;
		std::unordered_map<char32, vec2> m_Kerning;

		// addressing in texture
		uint8 m_Page = 0;
		uint8 m_Channel = 0;
		vec2 m_TexCoord = 0;
	};

	//---------------------------------
	// Charset
	//
	// a collection of metrics 
	//
	struct Charset
	{
		Charset() = default;
		Charset(char32 const start, char32 const end) : m_Start(start), m_End(end) {}

		char32 m_Start = 0u;
		char32 m_End = 0u; // inclusive
		std::vector<Metric> m_Characters;
	};

	// construct destruct
	//--------------------
	SdfFont() = default;
	~SdfFont() = default;

	SdfFont(SdfFont const& other);
	SdfFont& operator=(SdfFont const& other);

	// accessors
	//-----------
	std::string const& GetFamily() const { return m_FontFamily; }
	int16 GetFontSize() const { return m_FontSize; }
	bool IsBold() const { return m_Flags & E_Flags::F_Bold; }
	bool IsItalic() const { return m_Flags & E_Flags::F_Italic; }

	uint16 GetLineHeight() const { return m_LineHeight; }
	uint16 GetBaseline() const { return m_Baseline; }
	int16 GetUnderline() const { return m_Underline; }
	float GetUnderlineThickness() const { return m_UnderlineThickness; }

	Metric const* const GetMetric(char32 const character) const;
	bool UseKerning() const { return m_UseKerning; }

	render::TextureData const* GetAtlas() const { return (m_TextureAsset != nullptr) ? m_TextureAsset.get() : m_Texture.Get(); }


	// utility
	//---------
private:
	Metric* const GetMetric(char32 const character);

	// Data
	///////

	// font info
	std::string m_FontFamily;
	int16 m_FontSize = 0; // default size this sdf was generated from
	T_Flags m_Flags = E_Flags::F_None;

	uint16 m_LineHeight = 0u;
	uint16 m_Baseline = 0u;
	int16 m_Underline = 0;
	float m_UnderlineThickness = 0.f;

	// character info
	std::vector<Charset> m_CharSets;
	bool m_UseKerning = false;

	// texture info
	UniquePtr<render::TextureData const> m_Texture; // editor
	AssetPtr<render::TextureData> m_TextureAsset; // runtime
};

//---------------------------------
// SdfFontAsset
//
// Loadable Font Data
//
class SdfFontAsset final : public core::Asset<SdfFont, false>
{
	// definitions
	//-------------
	RTTR_ENABLE(core::Asset<SdfFont, false>)
	DECLARE_FORCED_LINKING()
public:
	static float const s_KerningAdjustment;

	// Construct destruct
	//---------------------
	SdfFontAsset() : core::Asset<SdfFont, false>() {}
	virtual ~SdfFontAsset() = default;

	// Asset overrides
	//---------------------
	bool LoadFromMemory(std::vector<uint8> const& data) override;

private:
	SdfFont* LoadFnt(std::vector<uint8> const& binaryContent);

	// Data
	///////
public:
	bool m_IsFallbackFont = false;
};


} // namespace gui
} // namespace et
