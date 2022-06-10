#pragma once
#include <RmlUi/Core/FontEngineInterface.h>
#include <RmlUi/Core/Texture.h>

#include <EtCore/Content/AssetPointer.h>

#include <EtGUI/Content/SdfFont.h>

#include "FontParameters.h"


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
	static Rml::FontEffectsHandle const s_DefaultEffects;

	static size_t const s_InvalidIdx;
	static size_t const s_NoIdx;

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
		FontFace(core::HashString const familyId, Rml::Style::FontStyle const style, SdfFont::E_Weight const weight, int32 const size);

		// immutable member vars
		core::HashString const m_FamilyId;
		Rml::Style::FontStyle const m_Style = Rml::Style::FontStyle::Normal;
		SdfFont::E_Weight const m_Weight = SdfFont::E_Weight::Auto;
		int32 const m_Size = 0;

		T_Hash const m_Hash; // computed from immutable vars

		// asset dependent member vars
		AssetPtr<SdfFont> m_Font;
		Rml::Texture m_Texture;
		int32 m_Version = -1;
		size_t m_FallbackIdx = s_InvalidIdx; // the index of the fallback font list that this face belongs to
		size_t m_NextFallbackFaceIdx = s_InvalidIdx; // points to another FontFace - forming a linked list of faces - computed on demand

		float m_Multiplier = 1.f; // initialize to 1 so there is a previous value when the asset is set

		int32 m_XHeight;
		int32 m_LineHeight;
		int32 m_Baseline;
		int32 m_Underline;
		float m_UnderlineThickness;

		float m_SdfSize;
		float m_SdfThreshold = 0.5;

		std::vector<size_t> m_LayerConfigurations;
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

		std::string m_Name;
		std::vector<size_t> m_FaceIndices; // faces can be addressed by index because they are never reordered
		std::vector<AssetPtr<SdfFont>> m_UniqueAssets;
		std::vector<Rml::Texture> m_AssetTextures;
	};

	typedef std::unordered_map<core::HashString, FontFamily> T_FontFamilies;

	//---------------------------------
	// LayerConfiguration
	//
	// A set of text layers containing parameters to render the font with
	//
	struct LayerConfiguration
	{
		std::vector<TextLayer> m_Layers;
		T_Hash m_Hash = 0u;
		size_t m_FaceIndex = s_InvalidIdx; // which font face this was generated for
		size_t m_MainLayerIdx = s_InvalidIdx;
	};

	typedef std::vector<LayerConfiguration> T_LayerConfigurations; // font effect handle is idx + 1


	// construct destruct
	//--------------------
public:
	RmlFontEngineInterface();
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
	void SetFaceAsset(FontFace& face, FontFamily const& family, size_t const assetIdx);
	size_t GetBestAssetForFace(FontFamily const& family, FontFace const& face) const;

	SdfFont::Metric const& GetMetric(size_t const faceIdx, char32 const charId, FontFace*& outFace);

	void AddFallbackFont(core::HashString const familyId, size_t const assetIdx);
	size_t GetFallbackFaceIdx(size_t const faceIdx);

	LayerConfiguration const& GetLayerConfiguration(Rml::FontEffectsHandle const effectsHandle) const;
	T_Hash GetLayerHash(std::vector<TextLayer> const& layers) const;
	TextLayer const& GetDefaultLayer() const;
	LayerConfiguration const& GetFallbackLayerConfig(LayerConfiguration const& inConfig, FontFace& fallbackFace);
	void ConvertLayerForNewFace(TextLayer& layer, float const prevSdfSize, float const newSdfSize);
	bool HasExistingLayerConfig(LayerConfiguration const& layerConfig, size_t& outConfigIdx);


	// Data
	///////

	T_FontFamilies m_Families;
	T_FallbackFonts m_FallbackFonts;

	T_FontFaces m_Faces;

	T_LayerConfigurations m_LayerConfigurations;
};


} // namespace gui
} // namespace et

