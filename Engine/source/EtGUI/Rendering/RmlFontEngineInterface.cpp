#include "stdafx.h"
#include "RmlFontEngineInterface.h"

#include <RmlUi/Core/StringUtilities.h>
#include <RmlUi/Core/FontEffect.h>

#include <EtCore/Content/ResourceManager.h>

#include <EtGUI/Context/RmlUtil.h>


namespace et {
namespace gui {


//==========================
// Font Engine :: Font Face
//==========================


//-----------------------------------------
// RmlFontEngineInterface::FontFace::c-tor
//
RmlFontEngineInterface::FontFace::FontFace(core::HashString const familyId,
	Rml::Style::FontStyle const style, 
	Rml::Style::FontWeight const weight, 
	int32 const size
)
	: m_FamilyId(familyId)
	, m_Style(style)
	, m_Weight(weight)
	, m_Size(size)
	, m_Hash(GetDataHash(reinterpret_cast<uint8 const*>(&m_FamilyId), sizeof(m_FamilyId) + sizeof(m_Style) + sizeof(m_Weight) + sizeof(m_Size)))
{ }

//--------------------------------------------
// RmlFontEngineInterface::FontFace::SetAsset
//
void RmlFontEngineInterface::FontFace::SetAsset(FontFamily const& family, 
	size_t const assetIdx, 
	RmlFontEngineInterface::T_FallbackFonts const& fallbackFonts)
{
	m_Font = family.m_UniqueAssets[assetIdx];
	m_Texture = family.m_AssetTextures[assetIdx];
	m_Version++;

	m_NextFallbackFaceIdx = s_InvalidIdx; // reset the fallback face
	auto const foundFallbackIt = std::find_if(fallbackFonts.cbegin(), fallbackFonts.cend(), [this, assetIdx](FallbackFont const& lhs)
		{
			return ((lhs.m_FamilyId == m_FamilyId) && (lhs.m_AssetIdx == assetIdx));
		});
	if (foundFallbackIt != fallbackFonts.cend())
	{
		m_FallbackIdx = static_cast<size_t>(foundFallbackIt - fallbackFonts.cbegin());
	}
	else
	{
		m_FallbackIdx = s_InvalidIdx;
	}

	m_Multiplier = static_cast<float>(m_Size) / static_cast<float>(m_Font->GetFontSize());

	SdfFont::Metric const* const xMetric = m_Font->GetValidMetric(static_cast<char32>('x'));
	ET_ASSERT(xMetric != nullptr);
	m_XHeight = static_cast<int32>(m_Multiplier * static_cast<float>(xMetric->m_Height));

	m_LineHeight = static_cast<int32>(m_Multiplier * static_cast<float>(m_Font->GetLineHeight()));
	m_Baseline = static_cast<int32>(m_Multiplier * static_cast<float>(m_Font->GetBaseline()));
	m_Underline = static_cast<int32>(m_Multiplier * static_cast<float>(m_Font->GetUnderline()));
	m_UnderlineThickness = static_cast<int32>(m_Multiplier * static_cast<float>(m_Font->GetUnderlineThickness()));

	m_SdfSize = m_Multiplier * m_Font->GetSdfSize();
}


//============================
// Font Engine :: Font Family
//============================


//--------------------------------------------------
// RmlFontEngineInterface::FontFamily::GetBestAsset
//
size_t RmlFontEngineInterface::FontFamily::GetBestAsset(RmlFontEngineInterface::FontFace const& face) const
{
	size_t bestMatch = s_InvalidIdx;
	uint32 bestScore = 0u;

	for (size_t assetIdx = 0u; assetIdx < m_UniqueAssets.size(); ++assetIdx)
	{
		AssetPtr<SdfFont> const asset = m_UniqueAssets[assetIdx];

		uint32 currentScore = 1u; // family should already match

		// matching style is much more important for choosing the asset than any other parameter
		Rml::Style::FontStyle const currentStyle = asset->IsItalic() ? Rml::Style::FontStyle::Italic : Rml::Style::FontStyle::Normal;
		if (currentStyle == face.m_Style)
		{
			currentScore += 40u;
		}

		// now we can find the closest match possible based on font size
		currentScore += static_cast<uint32>(30 - std::min(30, std::abs(static_cast<int32>(asset->GetFontSize()) - face.m_Size)));

		// and finally we can fetch the asset based on if it's bold or not (very easy to generate 
		Rml::Style::FontWeight const currentWeight = asset->IsItalic() ? Rml::Style::FontWeight::Bold : Rml::Style::FontWeight::Normal;
		if (currentWeight == face.m_Weight)
		{
			currentScore += 9u; // this could be a bit more sophisticated by making this relative to the weight difference
		}


		if (currentScore > bestScore)
		{
			bestScore = currentScore;
			bestMatch = assetIdx;
		}
	}

	ET_ASSERT(bestMatch != s_InvalidIdx);
	return bestMatch;
}


//===========================
// RML Font Engine Interface
//===========================


// static
Rml::FontEffectsHandle const RmlFontEngineInterface::s_InvalidEffects = 0u;
Rml::FontFaceHandle const RmlFontEngineInterface::s_InvalidFont = 0u;

size_t const RmlFontEngineInterface::s_InvalidIdx = std::numeric_limits<size_t>::max();
size_t const RmlFontEngineInterface::s_NoIdx = RmlFontEngineInterface::s_InvalidIdx - 1;


//--------------------------------------
// RmlFontEngineInterface::LoadFontFace
//
bool RmlFontEngineInterface::LoadFontFace(Rml::String const& fileName, bool const fallbackFace, Rml::Style::FontWeight const weight)
{
	UNUSED(weight); // can entirely be ignored since the weights are already generated in the asset

	core::HashString const assetId(fileName.c_str());

	AssetPtr<SdfFont> const font = core::ResourceManager::Instance()->GetAssetData<SdfFont>(assetId);
	if (font == nullptr)
	{
		return false;
	}

	core::HashString familyId;
	FontFamily& family = FindOrCreateFamily(font->GetFamily(), familyId);

	auto const assetIt = std::find(family.m_UniqueAssets.cbegin(), family.m_UniqueAssets.cend(), font);
	if (assetIt != family.m_UniqueAssets.cend())
	{
		if (fallbackFace)
		{
			AddFallbackFont(familyId, static_cast<size_t>(assetIt - family.m_UniqueAssets.cbegin()));
		}

		return true;
	}

	Rml::Texture texture;
	texture.Set(fileName); // render interface will grab the texture from the font asset - this also allows us to know to use the text shader
	family.m_UniqueAssets.push_back(std::move(font));
	family.m_AssetTextures.push_back(texture);
	if (fallbackFace)
	{
		AddFallbackFont(familyId, family.m_UniqueAssets.size() - 1);
	}

	for (size_t const faceIdx : family.m_FaceIndices)
	{
		ET_ASSERT(faceIdx < m_Faces.size());
		FontFace& face = m_Faces[faceIdx];

		face.SetAsset(family, family.GetBestAsset(face), m_FallbackFonts);
	}

	return true;
}

//--------------------------------------
// RmlFontEngineInterface::LoadFontFace
//
bool RmlFontEngineInterface::LoadFontFace(Rml::byte const* const data, 
	int32 const dataSize, 
	Rml::String const& familyName,
	Rml::Style::FontStyle const style, 
	Rml::Style::FontWeight const weight, 
	bool const fallbackFace)
{
	UNUSED(data);
	UNUSED(dataSize);
	UNUSED(style);
	UNUSED(weight);
	UNUSED(fallbackFace);

	if (familyName == "rmlui-debugger-font") // exception for the debugger font
	{
		ET_ASSERT(false, "Loading debug font from data is not supported");
		return false;
	}

	ET_ASSERT(false, "Loading Sdf fonts from data is not supported");
	return false;
}

//-------------------------------------------
// RmlFontEngineInterface::GetFontFaceHandle
//
Rml::FontFaceHandle RmlFontEngineInterface::GetFontFaceHandle(Rml::String const& familyName, 
	Rml::Style::FontStyle const style, 
	Rml::Style::FontWeight const weight, 
	int32 const size)
{
	FontFace face(core::HashString(familyName.c_str()), style, weight, size);

	auto const foundFaceIt = std::find_if(m_Faces.cbegin(), m_Faces.cend(), [&face](FontFace const& lh) 
		{
			return (lh.m_Hash == face.m_Hash);
		});

	if (foundFaceIt != m_Faces.cend())
	{
		return static_cast<Rml::FontFaceHandle>(foundFaceIt - m_Faces.cbegin()) + 1;
	}

	// find family
	T_FontFamilies::iterator const foundFamilyIt = m_Families.find(face.m_FamilyId);
	if (foundFamilyIt == m_Families.cend())
	{
		ET_ASSERT(false, "Family '%s' couldn't be found, failed to create handle", familyName.c_str());
		return s_InvalidFont;
	}

	FontFamily& family = foundFamilyIt->second;
	size_t const faceIdx = m_Faces.size();
	family.m_FaceIndices.push_back(faceIdx);

	face.SetAsset(family, family.GetBestAsset(face), m_FallbackFonts);

	// add to face list
	m_Faces.push_back(face);
	return static_cast<Rml::FontFaceHandle>(faceIdx + 1); // 0 is reserved for invalid
}

//--------------------------------------------
// RmlFontEngineInterface::PrepareFontEffects
//
Rml::FontEffectsHandle RmlFontEngineInterface::PrepareFontEffects(Rml::FontFaceHandle const faceHandle, Rml::FontEffectList const& fontEffects)
{
	UNUSED(faceHandle);

	for (std::shared_ptr<Rml::FontEffect const> const& effect : fontEffects)
	{
		//LOG(typeid(*effect).name());
	}

	return s_InvalidEffects;
}

//---------------------------------
// RmlFontEngineInterface::GetSize
//
int32 RmlFontEngineInterface::GetSize(Rml::FontFaceHandle const faceHandle)
{
	return GetFace(faceHandle).m_Size;
}

//------------------------------------
// RmlFontEngineInterface::GetXHeight
//
int32 RmlFontEngineInterface::GetXHeight(Rml::FontFaceHandle const faceHandle)
{
	return GetFace(faceHandle).m_XHeight;
}

//---------------------------------------
// RmlFontEngineInterface::GetLineHeight
//
int32 RmlFontEngineInterface::GetLineHeight(Rml::FontFaceHandle const faceHandle)
{
	return GetFace(faceHandle).m_LineHeight;
}

//-------------------------------------
// RmlFontEngineInterface::GetBaseline
//
int32 RmlFontEngineInterface::GetBaseline(Rml::FontFaceHandle const faceHandle)
{
	return GetFace(faceHandle).m_Baseline;
}

//--------------------------------------
// RmlFontEngineInterface::GetUnderline
//
float RmlFontEngineInterface::GetUnderline(Rml::FontFaceHandle const faceHandle, float& outThickness)
{
	FontFace const& face = GetFace(faceHandle);

	outThickness = face.m_UnderlineThickness;
	return face.m_Underline;
}

//----------------------------------------
// RmlFontEngineInterface::GetStringWidth
//
int32 RmlFontEngineInterface::GetStringWidth(Rml::FontFaceHandle const faceHandle, Rml::String const& utf8String, Rml::Character const priorCharacter)
{
	FontFace& face = GetFace(faceHandle);

	float stringWidth = 0.f;

	char32 prevChar = static_cast<char32>(priorCharacter);
	for (auto itString = Rml::StringIteratorU8(utf8String); itString; ++itString)
	{
		char32 const charId = static_cast<char32>(*itString);

		FontFace const* glyphFace = nullptr;
		SdfFont::Metric const& metric = GetMetric(face, charId, glyphFace);

		float kerning = 0.f;
		if (glyphFace->m_Font->UseKerning())
		{
			kerning = metric.GetKerningVec(prevChar).x;
		}

		prevChar = charId;

		stringWidth += (metric.m_AdvanceX + kerning) * glyphFace->m_Multiplier;
	}

	return static_cast<int32>(stringWidth);
}

//----------------------------------------
// RmlFontEngineInterface::GenerateString
//
int32 RmlFontEngineInterface::GenerateString(Rml::FontFaceHandle const faceHandle, 
	Rml::FontEffectsHandle const effectsHandle, 
	Rml::String const& utf8String,
	Rml::Vector2f const& position, 
	Rml::Colourb const& colour, 
	float const opacity, 
	Rml::GeometryList& outGeometry)
{
	UNUSED(effectsHandle);

	FontFace& inFace = GetFace(faceHandle);

	outGeometry = Rml::GeometryList();

	float stringWidth = 0.f; // ret

	// Sort text into glyphs per unique font face texture
	//----------------------------------------------------

	struct PerGlyph
	{
		PerGlyph(FontFace const& face, SdfFont::Metric const& metric, vec2 const pos) : m_Face(face), m_Metric(metric), m_Pos(pos) {}

		FontFace const& m_Face;
		SdfFont::Metric const& m_Metric;
		vec2 const m_Pos;
	};

	typedef std::pair<Rml::Texture, std::vector<PerGlyph>> T_PerTexture;

	std::vector<T_PerTexture> textureGlyphs;

	char32 prevChar = 0;
	for (auto itString = Rml::StringIteratorU8(utf8String); itString; ++itString)
	{
		char32 const charId = static_cast<char32>(*itString);

		FontFace const* glyphFace = nullptr;
		SdfFont::Metric const& metric = GetMetric(inFace, charId, glyphFace);

		vec2 kerning;
		if (glyphFace->m_Font->UseKerning())
		{
			kerning = metric.GetKerningVec(prevChar);
		}

		prevChar = charId;

		stringWidth += kerning.x * glyphFace->m_Multiplier;

		// insert glyph to its correlated container
		vec2 const pos(stringWidth, kerning.y * glyphFace->m_Multiplier);
		auto const foundIt = std::find_if(textureGlyphs.begin(), textureGlyphs.end(), [glyphFace](T_PerTexture const& lh)
			{
				return (lh.first == glyphFace->m_Texture);
			});
		if (foundIt == textureGlyphs.cend())
		{
			textureGlyphs.emplace_back(glyphFace->m_Texture, std::vector<PerGlyph>({ PerGlyph(*glyphFace, metric, pos) }));
		}
		else
		{
			foundIt->second.emplace_back(*glyphFace, metric, pos);
		}

		stringWidth += metric.m_AdvanceX * glyphFace->m_Multiplier;
	}

	// set up geometry per glyph texture
	//-----------------------------------

	vec2 const pos = RmlUtil::ToEtm(position);

	for (T_PerTexture const& perTexPair : textureGlyphs)
	{
		outGeometry.push_back(Rml::Geometry());
		Rml::Geometry& geometry = outGeometry.back();

		std::vector<Rml::Vertex>& vertices = geometry.GetVertices();
		std::vector<int32>& indices = geometry.GetIndices();

		std::vector<PerGlyph> const& glyphs = perTexPair.second;

		size_t const numCharacters = glyphs.size();
		ET_ASSERT(numCharacters > 0u);
		indices.reserve(numCharacters * 6);

		geometry.SetTexture(&glyphs[0u].m_Face.m_Texture); // set from the font face pointer because the perTexPair memory will go out of scope

		// Since RmlUi defines a preset vertex layout we're going to have to do some memory magic to also pass the channel indices to the render interface
		size_t const vCountBase = numCharacters * 4; // doubles as byte size of channel indices because they are 1 byte each
		size_t const channelVCount = (vCountBase + sizeof(Rml::Vertex) - 1) / sizeof(Rml::Vertex);

		// append enough vertices to provide a block of memory that we can repurpose for filling in the channels
		vertices.resize(vCountBase + channelVCount); 

		uint8* const channels = reinterpret_cast<uint8*>(&vertices[vCountBase]);

		Rml::Colourb col(colour.red, colour.green, colour.blue, static_cast<Rml::byte>(static_cast<float>(colour.alpha) * opacity));

		// fill in geometry
		//------------------

		vec2 const texDim = math::vecCast<float>(glyphs[0u].m_Face.m_Font->GetAtlas()->GetResolution());

		size_t vIndex = 0u;

		for (PerGlyph const& glyph : glyphs)
		{
			// indices, counter clockwise winding
			indices.push_back(static_cast<int32>(vIndex));
			indices.push_back(static_cast<int32>(vIndex + 2));
			indices.push_back(static_cast<int32>(vIndex + 1));
			indices.push_back(static_cast<int32>(vIndex + 1));
			indices.push_back(static_cast<int32>(vIndex + 2));
			indices.push_back(static_cast<int32>(vIndex + 3));

			// vertices
			vec2 charPos(pos + glyph.m_Pos + vec2(glyph.m_Metric.m_OffsetX, glyph.m_Metric.m_OffsetY) * glyph.m_Face.m_Multiplier);
			vec2 const charDim(static_cast<float>(glyph.m_Metric.m_Width), static_cast<float>(glyph.m_Metric.m_Height));
			vec2 charDimScaled = charDim * glyph.m_Face.m_Multiplier;

			vec2 const texCoord = glyph.m_Metric.m_TexCoord;
			vec2 const charDimTexture = (charDim / texDim);

			charPos = charPos + vec2(glyph.m_Face.m_SdfSize) * 0.5f;
			charDimScaled = charDimScaled - vec2(glyph.m_Face.m_SdfSize);//

			channels[vIndex] = glyph.m_Metric.m_Channel;
			Rml::Vertex& v1 = vertices[vIndex++];
			channels[vIndex] = glyph.m_Metric.m_Channel;
			Rml::Vertex& v2 = vertices[vIndex++];
			channels[vIndex] = glyph.m_Metric.m_Channel;
			Rml::Vertex& v3 = vertices[vIndex++];
			channels[vIndex] = glyph.m_Metric.m_Channel;
			Rml::Vertex& v4 = vertices[vIndex++];

			v1.position = Rml::Vector2f(charPos.x, charPos.y + charDimScaled.y);
			v1.colour = col;
			v1.tex_coord = Rml::Vector2f(texCoord.x, texCoord.y + charDimTexture.y);

			v2.position = Rml::Vector2f(charPos.x + charDimScaled.x, charPos.y + charDimScaled.y);
			v2.colour = col;
			v2.tex_coord = Rml::Vector2f(texCoord.x + charDimTexture.x, texCoord.y + charDimTexture.y);

			v3.position = Rml::Vector2f(charPos.x, charPos.y);
			v3.colour = col;
			v3.tex_coord = Rml::Vector2f(texCoord.x, texCoord.y);

			v4.position = Rml::Vector2f(charPos.x + charDimScaled.x, charPos.y);
			v4.colour = col;
			v4.tex_coord = Rml::Vector2f(texCoord.x + charDimTexture.x, texCoord.y);
		}
	}

	return static_cast<int32>(stringWidth);
}

//------------------------------------
// RmlFontEngineInterface::GetVersion
//
int32 RmlFontEngineInterface::GetVersion(Rml::FontFaceHandle const faceHandle)
{
	return GetFace(faceHandle).m_Version;
}

//----------------------------------------------
// RmlFontEngineInterface::ReleaseFontResources
//
void RmlFontEngineInterface::ReleaseFontResources()
{
	m_Faces.clear();
	m_Families.clear();
}


//----------------------------------------------
// RmlFontEngineInterface::FindOrCreateFamily
//
RmlFontEngineInterface::FontFamily& RmlFontEngineInterface::FindOrCreateFamily(std::string const& familyName, core::HashString& outFamilyId)
{
	outFamilyId = core::HashString(familyName.c_str());

	T_FontFamilies::iterator const foundIt = m_Families.find(outFamilyId);
	if (foundIt != m_Families.cend())
	{
		return foundIt->second;
	}

	auto const res = m_Families.emplace(outFamilyId, familyName);
	ET_ASSERT(res.second);

	return res.first->second;
}

//----------------------------------------------
// RmlFontEngineInterface::GetFace
//
RmlFontEngineInterface::FontFace& RmlFontEngineInterface::GetFace(Rml::FontFaceHandle const faceHandle)
{
	ET_ASSERT(faceHandle != s_InvalidFont);
	size_t const faceIdx = faceHandle - 1;

	ET_ASSERT(m_Faces.size() > faceIdx);
	return m_Faces[faceIdx];
}

//----------------------------------------------
// RmlFontEngineInterface::GetMetric
//
// Get the glyph metric from the face, or a fallback face if it couldn't be found. Sets outFace to the face the metric was found in
//
SdfFont::Metric const& RmlFontEngineInterface::GetMetric(RmlFontEngineInterface::FontFace& inFace, 
	char32 const charId, 
	RmlFontEngineInterface::FontFace const*& outFace)
{
	
	FontFace* currentFace = &inFace;
	while (currentFace != nullptr)
	{
		SdfFont::Metric const* metric = currentFace->m_Font->GetValidMetric(charId);
		if (metric != nullptr)
		{
			outFace = currentFace;
			return *metric;
		}

		currentFace = GetFallbackFace(*currentFace);
	}

	LOG(FS("FontEngine::GetMetric > Font '%s' doesn't support char '%#010x', and no fallbacks where found", inFace.m_Font.GetId().ToStringDbg(), charId),
		core::LogLevel::Warning);

	SdfFont::Metric const* metric = inFace.m_Font->GetValidMetric(0);
	ET_ASSERT(metric != nullptr); // every font should contain a default character for 0

	outFace = &inFace;
	return *metric;
}

//----------------------------------------------
// RmlFontEngineInterface::AddFallbackFont
//
void RmlFontEngineInterface::AddFallbackFont(core::HashString const familyId, size_t const assetIdx)
{
	FallbackFont const fnt(familyId, assetIdx);
	if (std::find_if(m_FallbackFonts.cbegin(), m_FallbackFonts.cend(), [&fnt](FallbackFont const& lhs)
		{
			return ((lhs.m_FamilyId == fnt.m_FamilyId) && (lhs.m_AssetIdx == fnt.m_AssetIdx));
		}) != m_FallbackFonts.cend())
	{
		m_FallbackFonts.push_back(fnt);
	}
}

//----------------------------------------------
// RmlFontEngineInterface::GetFallbackFace
//
// Lazy fetch the next face in the fallback chain, or nullptr if this is the last face
//
RmlFontEngineInterface::FontFace* RmlFontEngineInterface::GetFallbackFace(RmlFontEngineInterface::FontFace& face)
{
	if (face.m_NextFallbackFaceIdx == s_InvalidIdx) // lazy compute the pointer to the next face in the fallback chain
	{
		size_t const nextFallbackIdx = (face.m_FallbackIdx == s_InvalidIdx) ? 0u : face.m_FallbackIdx + 1;
		if (nextFallbackIdx >= m_FallbackFonts.size())
		{
			face.m_NextFallbackFaceIdx = s_NoIdx; // there are no further faces in the fallback chain
		}
		else // find or generate a font face for this particular fallback font
		{
			FallbackFont const& fallbackFont = m_FallbackFonts[nextFallbackIdx];

			// derive the specific SdfFont asset from the fallback font
			T_FontFamilies::iterator const foundFamilyIt = m_Families.find(fallbackFont.m_FamilyId);
			ET_ASSERT(foundFamilyIt != m_Families.cend());
			FontFamily& family = foundFamilyIt->second;

			AssetPtr<SdfFont> const asset = family.m_UniqueAssets[fallbackFont.m_AssetIdx];

			FontFace newFace(fallbackFont.m_FamilyId, face.m_Style, face.m_Weight, face.m_Size);
			auto const foundFaceIt = std::find_if(m_Faces.cbegin(), m_Faces.cend(), [&newFace, asset](FontFace const& lh)
				{
					return ((lh.m_Hash == newFace.m_Hash) && (lh.m_Font == asset));
				});

			if (foundFaceIt != m_Faces.cend()) 
			{
				// if we have a FontFace with the parameters AND specific asset matching the fallback font we don't need a new one
				ET_ASSERT(foundFaceIt->m_FallbackIdx == nextFallbackIdx); // should be assigned to this fallback font
				face.m_NextFallbackFaceIdx = static_cast<size_t>(foundFaceIt - m_Faces.cbegin());
			}
			else // otherwise we create a new font face for that family with the specific asset in the fallback font
			{
				newFace.SetAsset(family, fallbackFont.m_AssetIdx, m_FallbackFonts);

				face.m_NextFallbackFaceIdx = m_Faces.size();
				family.m_FaceIndices.push_back(face.m_NextFallbackFaceIdx);

				// add to face list
				m_Faces.push_back(newFace);
			}
		}
	}

	if (face.m_NextFallbackFaceIdx == s_NoIdx)
	{
		return nullptr;
	}

	ET_ASSERT(m_Faces.size() > face.m_NextFallbackFaceIdx);
	return &m_Faces[face.m_NextFallbackFaceIdx];
}


} // namespace gui
} // namespace et
