#pragma once
#include <RmlUi/Core/FontEngineInterface.h>
#include <RmlUi/Core/Texture.h>

#include <EtCore/Content/AssetPointer.h>

#include <EtGUI/Content/SdfFont.h>


namespace et {
namespace gui {


//---------------------------------
// RmlFontEngineInterface
//
// Font engine implementation for SDF fonts
//
class RmlFontEngineInterface final : public Rml::FontEngineInterface
{
	// definitions
	//-------------
	static Rml::FontFaceHandle const s_InvalidFont;
	static Rml::FontEffectsHandle const s_InvalidEffects;

	static size_t const s_InvalidIdx;
	static size_t const s_NoIdx;

	struct FontFamily;

	//---------------------------------
	// FallbackFont
	//
	// points into a font family to allow deriving a fallback face
	//
	struct FallbackFont
	{
		FallbackFont(core::HashString const familyId, size_t const assetIdx) : m_FamilyId(familyId), m_AssetIdx(assetIdx) {}

		core::HashString m_FamilyId;
		size_t const m_AssetIdx;
	};

	typedef std::vector<FallbackFont> T_FallbackFonts;

	//---------------------------------
	// FontFace
	//
	// An instance of a font family that links specific style, weight and size with a font asset
	//
	struct FontFace
	{
		FontFace(core::HashString const familyId, Rml::Style::FontStyle const style, Rml::Style::FontWeight const weight, int32 const size);

		void SetAsset(FontFamily const& family, size_t const assetIdx, T_FallbackFonts const& fallbackFonts);

		core::HashString const m_FamilyId;
		Rml::Style::FontStyle const m_Style = Rml::Style::FontStyle::Normal;
		Rml::Style::FontWeight const m_Weight = Rml::Style::FontWeight::Auto;
		int32 const m_Size = 0;

		T_Hash const m_Hash;

		// asset dependent variables
		AssetPtr<SdfFont> m_Font;
		Rml::Texture m_Texture;
		int32 m_Version = -1;
		size_t m_FallbackIdx = s_InvalidIdx; // the index of the fallback font list that this face belongs to
		size_t m_NextFallbackFaceIdx = s_InvalidIdx; // points to another FontFace - forming a linked list of faces - computed on demand

		float m_Multiplier;

		int32 m_XHeight;
		int32 m_LineHeight;
		int32 m_Baseline;
		int32 m_Underline;
		float m_UnderlineThickness;

		float m_SdfSize;
	};

	typedef std::vector<FontFace> T_FontFaces;

	//---------------------------------
	// FontFamily
	//
	// A collection of font assets and faces belonging to the same font
	//
	struct FontFamily
	{
		FontFamily() = default;
		FontFamily(std::string const& name) : m_Name(name) {}

		size_t GetBestAsset(FontFace const& face) const;

		std::string m_Name;
		std::vector<size_t> m_FaceIndices; // faces can be addressed by index because they are never reordered
		std::vector<AssetPtr<SdfFont>> m_UniqueAssets;
		std::vector<Rml::Texture> m_AssetTextures;
	};

	typedef std::unordered_map<core::HashString, FontFamily> T_FontFamilies;


	// construct destruct
	//--------------------
public:
	RmlFontEngineInterface() : Rml::FontEngineInterface() {}
	~RmlFontEngineInterface() = default;

	// interface
	//-----------
	bool LoadFontFace(Rml::String const& fileName, bool const fallbackFace, Rml::Style::FontWeight const weight) override;
	bool LoadFontFace(Rml::byte const* const data, 
		int32 const dataSize, 
		Rml::String const& familyName,
		Rml::Style::FontStyle const style,
		Rml::Style::FontWeight const weight, 
		bool const fallbackFace) override;

	Rml::FontFaceHandle GetFontFaceHandle(Rml::String const& familyName,
		Rml::Style::FontStyle const style, 
		Rml::Style::FontWeight const weight, 
		int32 const size) override;

	Rml::FontEffectsHandle PrepareFontEffects(Rml::FontFaceHandle const faceHandle, Rml::FontEffectList const& fontEffects) override;

	int32 GetSize(Rml::FontFaceHandle const faceHandle) override;
	int32 GetXHeight(Rml::FontFaceHandle const faceHandle) override;
	int32 GetLineHeight(Rml::FontFaceHandle const faceHandle) override;

	int32 GetBaseline(Rml::FontFaceHandle const faceHandle) override;

	float GetUnderline(Rml::FontFaceHandle const faceHandle, float& outThickness) override;

	int32 GetStringWidth(Rml::FontFaceHandle const faceHandle, Rml::String const& utf8String, Rml::Character const priorCharacter) override;

	int32 GenerateString(Rml::FontFaceHandle const faceHandle,
		Rml::FontEffectsHandle const effectsHandle, 
		Rml::String const& utf8String,
		Rml::Vector2f const& position,
		Rml::Colourb const& colour, 
		float const opacity, 
		Rml::GeometryList& outGeometry) override;

	int32 GetVersion(Rml::FontFaceHandle const faceHandle) override;

	void ReleaseFontResources() override;


	// utility
	//---------
private:
	FontFamily& FindOrCreateFamily(std::string const& familyName, core::HashString& outFamilyId);
	size_t GetFaceIdx(Rml::FontFaceHandle const faceHandle) const;
	FontFace& GetFace(Rml::FontFaceHandle const faceHandle);

	SdfFont::Metric const& GetMetric(size_t const faceIdx, char32 const charId, FontFace const*& outFace);

	void AddFallbackFont(core::HashString const familyId, size_t const assetIdx);
	size_t GetFallbackFaceIdx(size_t const faceIdx);


	// Data
	///////

	T_FontFamilies m_Families;
	T_FallbackFonts m_FallbackFonts;

	T_FontFaces m_Faces;
};


} // namespace gui
} // namespace et

