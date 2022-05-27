#pragma once
#include <RmlUi/Core/FontEngineInterface.h>

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

	struct FontFace
	{
		core::HashString m_Family;
		int32 m_Version = 0;

		Rml::Style::FontStyle m_Style = Rml::Style::FontStyle::Normal;
		Rml::Style::FontWeight m_Weight = Rml::Style::FontWeight::Auto;
		int32 m_Size = 0;
		AssetPtr<SdfFont> m_Font;
	};

	typedef std::vector<FontFace> T_FontFaces;

	struct FontFamily
	{
		std::string m_Name;
		std::vector<size_t> m_FaceIndices;
		std::vector<AssetPtr<SdfFont>> m_UniqueAssets;
	};

	typedef std::unordered_map<core::HashString, FontFamily> T_FontFamilies;
	typedef std::unordered_map<Rml::FontFaceHandle, size_t> T_FontFaceMap;

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
		Rml::String const& family, 
		Rml::Style::FontStyle const style,
		Rml::Style::FontWeight const weight, 
		bool const fallbackFace) override;

	Rml::FontFaceHandle GetFontFaceHandle(Rml::String const& family, 
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

	// Data
	///////

private:
};


} // namespace gui
} // namespace et

