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

	//---------------------------------
	// FontFace
	//
	// An instance of a font family that links specific style, weight and size with a font asset
	//
	struct FontFace
	{
		FontFace(core::HashString const familyId, Rml::Style::FontStyle const style, Rml::Style::FontWeight const weight, int32 const size);

		void SetAsset(AssetPtr<SdfFont> const asset, Rml::Texture const texture);

		core::HashString const m_FamilyId;
		Rml::Style::FontStyle const m_Style = Rml::Style::FontStyle::Normal;
		Rml::Style::FontWeight const m_Weight = Rml::Style::FontWeight::Auto;
		int32 const m_Size = 0;

		T_Hash const m_Hash;

		// asset dependent variables
		AssetPtr<SdfFont> m_Font;
		Rml::Texture m_Texture;
		int32 m_Version = -1;

		float m_Multiplier;

		int32 m_XHeight;
		int32 m_LineHeight;
		int32 m_Baseline;
		int32 m_Underline;
		float m_UnderlineThickness;
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

		AssetPtr<SdfFont> GetBestAsset(FontFace const& face, Rml::Texture& outTexture) const;

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

	int32 GetStringWidth(Rml::FontFaceHandle const faceHandle, Rml::String const& string, Rml::Character const priorCharacter) override;

	int32 GenerateString(Rml::FontFaceHandle const faceHandle,
		Rml::FontEffectsHandle const effectsHandle, 
		Rml::String const& string, 
		Rml::Vector2f const& position,
		Rml::Colourb const& colour, 
		float const opacity, 
		Rml::GeometryList& outGeometry) override;

	int32 GetVersion(Rml::FontFaceHandle const faceHandle) override;

	void ReleaseFontResources() override;


	// utility
	//---------
private:
	FontFamily& FindOrCreateFamily(std::string const& familyName);
	FontFace& GetFace(Rml::FontFaceHandle const faceHandle);


	// Data
	///////

	T_FontFamilies m_Families;

	T_FontFaces m_Faces;
};


} // namespace gui
} // namespace et

