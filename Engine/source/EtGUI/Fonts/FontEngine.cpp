#include "stdafx.h"
#include "FontEngine.h"

#include <RmlUi/Core/StringUtilities.h>
#include <RmlUi/Core/FontEffect.h>

#include <EtCore/Content/ResourceManager.h>

#include <EtGUI/Context/RmlUtil.h>

#include "FontParameters.h"
#include "FontEffects.h"


namespace et {
namespace gui {


//==========================
// Font Engine :: Font Face
//==========================


//-----------------------------------------
// FontEngine::FontFace::c-tor
//
FontEngine::FontFace::FontFace(core::HashString const familyId, Rml::Style::FontStyle const style, SdfFont::E_Weight const weight, int32 const size)
	: m_FamilyId(familyId)
	, m_Style(style)
	, m_Weight(weight)
	, m_Size(size)
	, m_Hash(GetDataHash(reinterpret_cast<uint8 const*>(&m_FamilyId), sizeof(m_FamilyId) + sizeof(m_Style) + sizeof(m_Weight) + sizeof(m_Size)))
{ }


//=============
// Font Engine 
//=============


// static
Rml::FontEffectsHandle const FontEngine::s_DefaultEffects = 0u;
Rml::FontFaceHandle const FontEngine::s_InvalidFont = 0u;

size_t const FontEngine::s_InvalidIdx = std::numeric_limits<size_t>::max();
size_t const FontEngine::s_NoIdx = FontEngine::s_InvalidIdx - 1;


//--------------------------------------
// FontEngine::c-tor
//
FontEngine::FontEngine() 
	: Rml::FontEngineInterface()
{
	// create default font effect layers - so an invalid font effect handle points to the default font layers
	m_LayerConfigurations.push_back(LayerConfiguration());
	LayerConfiguration& defaultConfig = m_LayerConfigurations.back();

	defaultConfig.m_Layers.push_back(TextLayer());
	defaultConfig.m_Hash = GetLayerHash(defaultConfig.m_Layers);
	defaultConfig.m_MainLayerIdx = 0u;
}

//--------------------------------------
// FontEngine::LoadFontFace
//
bool FontEngine::LoadFontFace(Rml::String const& fileName, bool const fallbackFace, Rml::Style::FontWeight const weight)
{
	ET_UNUSED(weight); // can entirely be ignored since the weights are already generated in the asset

	core::HashString const assetId(fileName.c_str());

	AssetPtr<SdfFont> const font = core::ResourceManager::Instance()->GetAssetData<SdfFont>(assetId);
	if (font == nullptr)
	{
		return false;
	}

	return LoadFontFaceInternal(font, font->GetFamily(), fallbackFace, fileName);
}

//--------------------------------------
// FontEngine::LoadFontFace
//
bool FontEngine::LoadFontFace(Rml::byte const* const data, 
	int32 const dataSize, 
	Rml::String const& familyName,
	Rml::Style::FontStyle const style, 
	Rml::Style::FontWeight const weight, 
	bool const fallbackFace)
{
	ET_UNUSED(data);
	ET_UNUSED(dataSize);
	ET_UNUSED(style);
	ET_UNUSED(weight);

	// exception for the debugger font
	static std::string const s_DebugFntFamilyName("rmlui-debugger-font");
	if (familyName == s_DebugFntFamilyName) 
	{
		AssetPtr<gui::SdfFont> const debugFont = core::ResourceManager::Instance()->GetAssetData<gui::SdfFont>(core::HashString("Fonts/IBMPlexMono.ttf"));
		ET_ASSERT(debugFont != nullptr);

		core::I_Asset const* const fntAsset = debugFont.GetAsset();
		std::string const fileName = fntAsset->GetPath() + fntAsset->GetName();

		return LoadFontFaceInternal(debugFont, s_DebugFntFamilyName, fallbackFace, fileName);
	}

	ET_ASSERT(false, "Loading Sdf fonts from data is not supported");
	return false;
}

//-------------------------------------------
// FontEngine::GetFontFaceHandle
//
Rml::FontFaceHandle FontEngine::GetFontFaceHandle(Rml::String const& familyName, 
	Rml::Style::FontStyle const style, 
	Rml::Style::FontWeight const weight, 
	int32 const size)
{
	FontFace face(core::HashString(familyName.c_str()), style, static_cast<SdfFont::E_Weight>(weight), size);

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

	SetFaceAsset(face, family, GetBestAssetForFace(family, face));

	// add to face list
	m_Faces.push_back(face);
	return static_cast<Rml::FontFaceHandle>(faceIdx + 1); // 0 is reserved for invalid
}

//--------------------------------------------
// FontEngine::PrepareFontEffects
//
Rml::FontEffectsHandle FontEngine::PrepareFontEffects(Rml::FontFaceHandle const faceHandle, Rml::FontEffectList const& fontEffects)
{
	if (fontEffects.empty()) // if there are no effects we refer to the default layer configuration
	{
		return s_DefaultEffects;
	}

	LayerConfiguration newConfig;
	newConfig.m_FaceIndex = GetFaceIdx(faceHandle);
	
	FontFace& face = m_Faces[newConfig.m_FaceIndex];

	// figure out what layers we need from these effects
	for (std::shared_ptr<Rml::FontEffect const> const& effect : fontEffects)
	{
		FontEffectBase const* const effectBase = dynamic_cast<FontEffectBase const*>(effect.get()); 
		if (effectBase != nullptr) // for safety, in case the Rml effect implementations are not overridden
		{
			if ((newConfig.m_MainLayerIdx == s_InvalidIdx) && (effect->GetLayer() == Rml::FontEffect::Layer::Front))
			{
				newConfig.m_MainLayerIdx = newConfig.m_Layers.size();
				newConfig.m_Layers.push_back(GetDefaultLayer());
			}

			FontEffectBase const* const effectBase = static_cast<FontEffectBase const*>(effect.get());
			TextLayer layer;
			effectBase->PrepareTextLayer(face.m_EffectMultiplier, layer);
			newConfig.m_Layers.push_back(layer);
		}
		else
		{
			// typeid contains more accurate type name for unregistered types
			LOG(FS("/tFont effect '%s' is not supported by SDF font engine", typeid(*effect).name())); 
		}
	}

	if (newConfig.m_Layers.empty()) // again go to default because we had no valid effects
	{
		return s_DefaultEffects;
	}

	if (newConfig.m_MainLayerIdx == s_InvalidIdx)
	{
		newConfig.m_MainLayerIdx = newConfig.m_Layers.size();
		newConfig.m_Layers.push_back(GetDefaultLayer());
	}

	newConfig.m_Hash = GetLayerHash(newConfig.m_Layers);

	// check if we already have a set of layers matching this configuration, and return a handle of that
	{
		size_t existingIdx;
		if (HasExistingLayerConfig(newConfig, existingIdx))
		{
			return static_cast<Rml::FontEffectsHandle>(existingIdx);
		}
	}

	// we didn't have a matching layer configuration, so instead we create a new one
	size_t const ret = m_LayerConfigurations.size();
	m_LayerConfigurations.push_back(std::move(newConfig));

	face.m_LayerConfigurations.push_back(ret); // register with the face in case it's asset changes

	return static_cast<Rml::FontEffectsHandle>(ret);
}

//---------------------------------
// FontEngine::GetSize
//
int32 FontEngine::GetSize(Rml::FontFaceHandle const faceHandle)
{
	return GetFace(faceHandle).m_Size;
}

//------------------------------------
// FontEngine::GetXHeight
//
int32 FontEngine::GetXHeight(Rml::FontFaceHandle const faceHandle)
{
	return GetFace(faceHandle).m_XHeight;
}

//---------------------------------------
// FontEngine::GetLineHeight
//
int32 FontEngine::GetLineHeight(Rml::FontFaceHandle const faceHandle)
{
	return GetFace(faceHandle).m_LineHeight;
}

//-------------------------------------
// FontEngine::GetBaseline
//
int32 FontEngine::GetBaseline(Rml::FontFaceHandle const faceHandle)
{
	return GetFace(faceHandle).m_Baseline;
}

//--------------------------------------
// FontEngine::GetUnderline
//
float FontEngine::GetUnderline(Rml::FontFaceHandle const faceHandle, float& outThickness)
{
	FontFace const& face = GetFace(faceHandle);

	outThickness = face.m_UnderlineThickness;
	return face.m_Underline;
}

//----------------------------------------
// FontEngine::GetStringWidth
//
int32 FontEngine::GetStringWidth(Rml::FontFaceHandle const faceHandle, Rml::String const& utf8String, Rml::Character const priorCharacter)
{
	size_t const faceIdx = GetFaceIdx(faceHandle);

	int32 stringWidth = 0;

	char32 prevChar = static_cast<char32>(priorCharacter);
	for (auto itString = Rml::StringIteratorU8(utf8String); itString; ++itString)
	{
		char32 const charId = static_cast<char32>(*itString);

		FontFace* glyphFace = nullptr;
		SdfFont::Metric const& metric = GetMetric(faceIdx, charId, glyphFace);

		float kerning = 0.f;
		if (glyphFace->m_Font->UseKerning())
		{
			kerning = metric.GetKerningVec(prevChar).x;
		}

		prevChar = charId;

		stringWidth += static_cast<int32>(((metric.m_AdvanceX + kerning) * glyphFace->m_Multiplier) + 0.5f);
	}

	return stringWidth;
}

//----------------------------------------
// FontEngine::GenerateString
//
int32 FontEngine::GenerateString(Rml::FontFaceHandle const faceHandle, 
	Rml::FontEffectsHandle const effectsHandle, 
	Rml::String const& utf8String,
	Rml::Vector2f const& position, 
	Rml::Colourb const& colour, 
	float const opacity, 
	Rml::GeometryList& outGeometry)
{
	size_t const faceIdx = GetFaceIdx(faceHandle);
	size_t const layerConfigIdx = GetLayerConfigIdx(effectsHandle);

	outGeometry = Rml::GeometryList();

	int32 stringWidth = 0; // ret

	// Sort text into glyphs per unique font face 
	//--------------------------------------------

	struct PerGlyph
	{
		PerGlyph(SdfFont::Metric const& metric, vec2 const pos) : m_Metric(metric), m_Pos(pos) {}

		SdfFont::Metric const& m_Metric;
		vec2 const m_Pos;
	};

	struct PerFace
	{
		PerFace(FontFace const* const face, size_t const layerConfigIdx, PerGlyph const& initialGlyph)
			: m_Face(face), m_LayerConfigIdx(layerConfigIdx), m_Glyphs({ initialGlyph }) {}

		FontFace const* const m_Face;
		size_t const m_LayerConfigIdx;

		std::vector<PerGlyph> m_Glyphs;
	};

	std::vector<PerFace> faceGlyphs;

	char32 prevChar = 0;
	for (auto itString = Rml::StringIteratorU8(utf8String); itString; ++itString)
	{
		char32 const charId = static_cast<char32>(*itString);

		FontFace* glyphFace = nullptr;
		SdfFont::Metric const& metric = GetMetric(faceIdx, charId, glyphFace);

		vec2 kerning;
		if (glyphFace->m_Font->UseKerning())
		{
			kerning = metric.GetKerningVec(prevChar);
		}

		prevChar = charId;

		float width = static_cast<float>(stringWidth);
		width += kerning.x * glyphFace->m_Multiplier; // casting back and forth to have consistency with GetStringWidth function

		// insert glyph to its correlated container
		vec2 const pos(width, kerning.y * glyphFace->m_Multiplier);
		auto const foundIt = std::find_if(faceGlyphs.begin(), faceGlyphs.end(), [glyphFace](PerFace const& lh)
			{
				return (lh.m_Face == glyphFace);
			});
		if (foundIt == faceGlyphs.cend())
		{
			// converting the default effects handle isn't necessary because it will only ever result in more effects that are the same
			size_t const faceLayerConfigIdx = ((glyphFace == &m_Faces[faceIdx]) || effectsHandle == s_DefaultEffects) ?  
				layerConfigIdx : GetFallbackLayerConfigIdx(layerConfigIdx, *glyphFace);
			faceGlyphs.emplace_back(glyphFace, faceLayerConfigIdx, PerGlyph(metric, pos));
		}
		else
		{
			foundIt->m_Glyphs.emplace_back(metric, pos);
		}

		width += metric.m_AdvanceX * glyphFace->m_Multiplier;
		stringWidth = static_cast<int32>(width + 0.5f); // rounding properly
	}

	// set up geometry per glyph texture
	//-----------------------------------

	vec2 const pos = RmlUtil::ToEtm(position);
	Rml::Colourb col(colour.red, colour.green, colour.blue, static_cast<Rml::byte>(static_cast<float>(colour.alpha) * opacity));
	vec4 colF(static_cast<float>(col.red) / 255.f, 
		static_cast<float>(col.green) / 255.f, 
		static_cast<float>(col.blue) / 255.f, 
		static_cast<float>(col.alpha) / 255.f);

	for (PerFace const& perFace : faceGlyphs)
	{
		outGeometry.push_back(Rml::Geometry());
		Rml::Geometry& geometry = outGeometry.back();

		std::vector<Rml::Vertex>& vertices = geometry.GetVertices();
		std::vector<int32>& indices = geometry.GetIndices();

		size_t const numCharacters = perFace.m_Glyphs.size();
		ET_ASSERT(numCharacters > 0u);
		indices.reserve(numCharacters * 6);

		geometry.SetTexture(&perFace.m_Face->m_Texture); // set from the font face pointer because the perFace memory will go out of scope

		// Since RmlUi defines a preset vertex layout we're going to have to do some memory magic to also pass the channel indices to the render interface
		size_t const vCountBase = numCharacters * 4; // doubles as byte size of channel indices because they are 1 byte each
		size_t const paramVCount = FontParameters::GetVCount();
		size_t const rmlVCount = ((vCountBase * sizeof(TextVertex)) + sizeof(Rml::Vertex) - 1) / sizeof(Rml::Vertex);

		// append enough vertices to provide a block of memory that we can repurpose for filling in the channels
		vertices.resize(paramVCount + rmlVCount);

		FontParameters& params = *reinterpret_cast<FontParameters*>(&vertices[0]);
		TextVertex* const textVertices = reinterpret_cast<TextVertex*>(&vertices[paramVCount]);

		LayerConfiguration const& layerConfig = m_LayerConfigurations[perFace.m_LayerConfigIdx];
		params.m_SdfThreshold = perFace.m_Face->m_SdfThreshold;
		params.m_LayerCount = layerConfig.m_Layers.size();
		params.m_Layers = ToPtr(layerConfig.m_Layers.data());
		params.m_MainLayerIdx = layerConfig.m_MainLayerIdx;
		params.m_MainLayerColor = colF;

		// fill in geometry
		//------------------

		vec2 const texDim = math::vecCast<float>(perFace.m_Face->m_Font->GetAtlas()->GetResolution());

		size_t vIndex = 0u;
		for (PerGlyph const& glyph : perFace.m_Glyphs)
		{
			// indices, counter clockwise winding
			indices.push_back(static_cast<int32>(vIndex));
			indices.push_back(static_cast<int32>(vIndex + 2));
			indices.push_back(static_cast<int32>(vIndex + 1));
			indices.push_back(static_cast<int32>(vIndex + 1));
			indices.push_back(static_cast<int32>(vIndex + 2));
			indices.push_back(static_cast<int32>(vIndex + 3));

			// vertices
			vec2 charPos(pos + glyph.m_Pos + vec2(glyph.m_Metric.m_OffsetX, glyph.m_Metric.m_OffsetY) * perFace.m_Face->m_Multiplier);
			vec2 const charDim(static_cast<float>(glyph.m_Metric.m_Width), static_cast<float>(glyph.m_Metric.m_Height));
			vec2 charDimScaled = charDim * perFace.m_Face->m_Multiplier;

			vec2 const texCoord = glyph.m_Metric.m_TexCoord;
			vec2 const charDimTexture = (charDim / texDim);

			charPos = charPos + vec2(perFace.m_Face->m_SdfSize) * 0.5f;
			charDimScaled = charDimScaled - vec2(perFace.m_Face->m_SdfSize);

			TextVertex& v1 = textVertices[vIndex++];
			TextVertex& v2 = textVertices[vIndex++];
			TextVertex& v3 = textVertices[vIndex++];
			TextVertex& v4 = textVertices[vIndex++];

			v1.m_Position = vec2(charPos.x, charPos.y + charDimScaled.y);
			v1.m_TexCoord = vec2(texCoord.x, texCoord.y + charDimTexture.y);
			v1.m_Channel = glyph.m_Metric.m_Channel;

			v2.m_Position = vec2(charPos.x + charDimScaled.x, charPos.y + charDimScaled.y);
			v2.m_TexCoord = vec2(texCoord.x + charDimTexture.x, texCoord.y + charDimTexture.y);
			v2.m_Channel = glyph.m_Metric.m_Channel;

			v3.m_Position = vec2(charPos.x, charPos.y);
			v3.m_TexCoord = vec2(texCoord.x, texCoord.y);
			v3.m_Channel = glyph.m_Metric.m_Channel;

			v4.m_Position = vec2(charPos.x + charDimScaled.x, charPos.y);
			v4.m_TexCoord = vec2(texCoord.x + charDimTexture.x, texCoord.y);
			v4.m_Channel = glyph.m_Metric.m_Channel;
		}
	}

	return stringWidth;
}

//------------------------------------
// FontEngine::GetVersion
//
int32 FontEngine::GetVersion(Rml::FontFaceHandle const faceHandle)
{
	return GetFace(faceHandle).m_Version;
}

//----------------------------------------------
// FontEngine::ReleaseFontResources
//
void FontEngine::ReleaseFontResources()
{
	m_Faces.clear();
	m_Families.clear();
}


//----------------------------------------------
// FontEngine::LoadFontFaceInternal
//
bool FontEngine::LoadFontFaceInternal(AssetPtr<SdfFont> const font, std::string const& familyName, bool const fallbackFace, std::string const& fileName)
{
	core::HashString familyId;
	FontFamily& family = FindOrCreateFamily(familyName, familyId);

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

		SetFaceAsset(face, family, GetBestAssetForFace(family, face));
	}

	return true;
}

//----------------------------------------------
// FontEngine::FindOrCreateFamily
//
FontEngine::FontFamily& FontEngine::FindOrCreateFamily(std::string const& familyName, core::HashString& outFamilyId)
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
// FontEngine::GetFaceIdx
//
size_t FontEngine::GetFaceIdx(Rml::FontFaceHandle const faceHandle) const
{
	ET_ASSERT(faceHandle != s_InvalidFont);
	size_t const faceIdx = faceHandle - 1;

	ET_ASSERT(m_Faces.size() > faceIdx);
	return faceIdx;
}

//----------------------------------------------
// FontEngine::GetFace
//
FontEngine::FontFace& FontEngine::GetFace(Rml::FontFaceHandle const faceHandle)
{
	return m_Faces[GetFaceIdx(faceHandle)];
}

//----------------------------------------------
// FontEngine::SetFaceAsset
//
void FontEngine::SetFaceAsset(FontFace& face, FontFamily const& family, size_t const assetIdx) 
{
	if (family.m_UniqueAssets[assetIdx] == face.m_Font)
	{
		return;
	}

	face.m_Font = family.m_UniqueAssets[assetIdx];
	face.m_Texture = family.m_AssetTextures[assetIdx];
	face.m_Version++;

	face.m_NextFallbackFaceIdx = s_InvalidIdx; // reset the fallback face
	auto const foundFallbackIt = std::find_if(m_FallbackFonts.cbegin(), m_FallbackFonts.cend(), [face, assetIdx](FallbackFont const& lhs)
		{
			return ((lhs.m_FamilyId == face.m_FamilyId) && (lhs.m_AssetIdx == assetIdx));
		});
	if (foundFallbackIt != m_FallbackFonts.cend())
	{
		face.m_FallbackIdx = static_cast<size_t>(foundFallbackIt - m_FallbackFonts.cbegin());
	}
	else
	{
		face.m_FallbackIdx = s_InvalidIdx;
	}

	face.m_Multiplier = static_cast<float>(face.m_Size) / static_cast<float>(face.m_Font->GetFontSize());

	SdfFont::Metric const* const xMetric = face.m_Font->GetValidMetric(static_cast<char32>('x'));
	ET_ASSERT(xMetric != nullptr);
	face.m_XHeight = static_cast<int32>(face.m_Multiplier * static_cast<float>(xMetric->m_Height));

	face.m_LineHeight = static_cast<int32>(face.m_Multiplier * static_cast<float>(face.m_Font->GetLineHeight()));
	face.m_Baseline = static_cast<int32>(face.m_Multiplier * static_cast<float>(face.m_Font->GetBaseline()));
	face.m_Underline = static_cast<int32>(face.m_Multiplier * static_cast<float>(face.m_Font->GetUnderline()));
	face.m_UnderlineThickness = face.m_Multiplier * static_cast<float>(face.m_Font->GetUnderlineThickness());

	face.m_SdfSize = face.m_Multiplier * face.m_Font->GetSdfSize();
	face.m_SdfThreshold = 0.5f + (static_cast<float>(face.m_Font->GetWeight()) - static_cast<float>(face.m_Weight)) * face.m_Font->GetThresholdPerWeight();

	float const prevMultiplier = face.m_EffectMultiplier;
	float const invSdfSize = (1.f / face.m_Multiplier) / face.m_Font->GetSdfSize();
	face.m_EffectMultiplier = 0.5f / face.m_SdfSize;

	// convert asset dependent text layer variables
	for (size_t const configIdx : face.m_LayerConfigurations)
	{
		LayerConfiguration& config = m_LayerConfigurations[configIdx];
		for (TextLayer& layer : config.m_Layers)
		{
			ConvertLayerForNewFace(layer, prevMultiplier, face.m_EffectMultiplier);
		}
	}
}

//----------------------------------------------
// FontEngine::GetBestAssetForFace
//
size_t FontEngine::GetBestAssetForFace(FontFamily const& family, FontFace const& face) const
{
	size_t bestMatch = s_InvalidIdx;
	uint32 bestScore = 0u;

	for (size_t assetIdx = 0u; assetIdx < family.m_UniqueAssets.size(); ++assetIdx)
	{
		AssetPtr<SdfFont> const asset = family.m_UniqueAssets[assetIdx];

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
		currentScore += static_cast<uint32>(9 - std::min(9, std::abs(static_cast<int32>(asset->GetWeight()) - static_cast<int32>(face.m_Weight)) / 100));

		if (currentScore > bestScore)
		{
			bestScore = currentScore;
			bestMatch = assetIdx;
		}
	}

	ET_ASSERT(bestMatch != s_InvalidIdx);
	return bestMatch;
}

//----------------------------------------------
// FontEngine::GetMetric
//
// Get the glyph metric from the face, or a fallback face if it couldn't be found. Sets outFace to the face the metric was found in
//
SdfFont::Metric const& FontEngine::GetMetric(size_t const faceIdx, char32 const charId, FontEngine::FontFace*& outFace)
{
	size_t currentFaceIdx = faceIdx;
	while (currentFaceIdx != s_InvalidIdx)
	{
		SdfFont::Metric const* metric = m_Faces[currentFaceIdx].m_Font->GetValidMetric(charId);
		if (metric != nullptr)
		{
			outFace = &m_Faces[currentFaceIdx];
			return *metric;
		}

		currentFaceIdx = GetFallbackFaceIdx(currentFaceIdx);
	}

	LOG(FS("FontEngine::GetMetric > Font '%s' doesn't support char '%#010x', and no fallbacks where found", 
		m_Faces[faceIdx].m_Font.GetId().ToStringDbg(), charId), core::LogLevel::Warning);

	SdfFont::Metric const* metric = m_Faces[faceIdx].m_Font->GetValidMetric(0);
	ET_ASSERT(metric != nullptr); // every font should contain a default character for 0

	outFace = &m_Faces[faceIdx];
	return *metric;
}

//----------------------------------------------
// FontEngine::AddFallbackFont
//
void FontEngine::AddFallbackFont(core::HashString const familyId, size_t const assetIdx)
{
	FallbackFont const fnt(familyId, assetIdx);
	if (std::find_if(m_FallbackFonts.cbegin(), m_FallbackFonts.cend(), [&fnt](FallbackFont const& lhs)
		{
			return ((lhs.m_FamilyId == fnt.m_FamilyId) && (lhs.m_AssetIdx == fnt.m_AssetIdx));
		}) == m_FallbackFonts.cend())
	{
		m_FallbackFonts.push_back(fnt);
	}
}

//----------------------------------------------
// FontEngine::GetFallbackFace
//
// Lazy fetch the next face in the fallback chain, or nullptr if this is the last face
//
size_t FontEngine::GetFallbackFaceIdx(size_t const faceIdx)
{
	FontFace const& face = m_Faces[faceIdx];
	if (face.m_NextFallbackFaceIdx == s_InvalidIdx) // lazy compute the pointer to the next face in the fallback chain
	{
		size_t const nextFallbackIdx = (face.m_FallbackIdx == s_InvalidIdx) ? 0u : face.m_FallbackIdx + 1;
		if (nextFallbackIdx >= m_FallbackFonts.size())
		{
			m_Faces[faceIdx].m_NextFallbackFaceIdx = s_NoIdx; // there are no further faces in the fallback chain
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
				m_Faces[faceIdx].m_NextFallbackFaceIdx = static_cast<size_t>(foundFaceIt - m_Faces.cbegin());
			}
			else // otherwise we create a new font face for that family with the specific asset in the fallback font
			{
				SetFaceAsset(newFace, family, fallbackFont.m_AssetIdx);
				size_t const newIdx = m_Faces.size();

				family.m_FaceIndices.push_back(newIdx);

				// add to face list
				m_Faces.push_back(newFace); // invalidates face ref access with []operator from here

				m_Faces[faceIdx].m_NextFallbackFaceIdx = newIdx;
			}
		}
	}

	size_t const nextFallback = m_Faces[faceIdx].m_NextFallbackFaceIdx;
	if (nextFallback == s_NoIdx)
	{
		return s_InvalidIdx;
	}

	ET_ASSERT(m_Faces.size() > nextFallback);
	return nextFallback;
}

//-------------------------------------------
// FontEngine::GetLayerConfigIdx
//
size_t FontEngine::GetLayerConfigIdx(Rml::FontEffectsHandle const effectsHandle) const
{
	size_t const layerIdx = static_cast<size_t>(effectsHandle);
	ET_ASSERT(layerIdx < m_LayerConfigurations.size());
	return layerIdx;
}

//-----------------------------------------------
// FontEngine::GetLayerConfiguration
//
FontEngine::LayerConfiguration const& FontEngine::GetLayerConfiguration(Rml::FontEffectsHandle const effectsHandle) const
{
	return m_LayerConfigurations[GetLayerConfigIdx(effectsHandle)];
}

//--------------------------------------
// FontEngine::GetLayerHash
//
T_Hash FontEngine::GetLayerHash(std::vector<TextLayer> const& layers) const
{
	return GetDataHash(reinterpret_cast<uint8 const*>(layers.data()), layers.size() * sizeof(TextLayer));
}

//-----------------------------------------
// FontEngine::GetDefaultLayer
//
// stored at index 0, because those are the default layers for text without effects
//
TextLayer const& FontEngine::GetDefaultLayer() const
{
	LayerConfiguration const& defaultConfig = GetLayerConfiguration(s_DefaultEffects);

	ET_ASSERT(defaultConfig.m_Layers.size() == 1u);
	return defaultConfig.m_Layers[0];
}

//---------------------------------------------------
// FontEngine::GetFallbackLayerConfigIdx
//
// find or generate a new layer configuration for a fallback face from an existing layer configuration that was made for a different font face
//
size_t FontEngine::GetFallbackLayerConfigIdx(size_t const inConfigIdx, FontFace& fallbackFace)
{
	LayerConfiguration const& inConfig = m_LayerConfigurations[inConfigIdx];
	FontFace const& inFace = m_Faces[inConfig.m_FaceIndex];

	// convert from existing layer configuration
	LayerConfiguration newConfig;
	for (TextLayer const& inLayer : inConfig.m_Layers)
	{
		newConfig.m_Layers.push_back(inLayer);
		ConvertLayerForNewFace(newConfig.m_Layers.back(), inFace.m_EffectMultiplier, fallbackFace.m_EffectMultiplier);
	}

	newConfig.m_MainLayerIdx = inConfig.m_MainLayerIdx;
	newConfig.m_Hash = GetLayerHash(newConfig.m_Layers);
	newConfig.m_FaceIndex = static_cast<size_t>(&fallbackFace - m_Faces.data()); // deduce fallbackFace index
	ET_ASSERT(newConfig.m_FaceIndex < m_Faces.size());

	// see if we already have a matching layer configuration
	{
		size_t existingIdx;
		if (HasExistingLayerConfig(newConfig, existingIdx))
		{
			return existingIdx;
		}
	}

	// we didn't have a matching layer configuration, so instead we create a new one
	size_t const newConfigIdx = m_LayerConfigurations.size();
	m_LayerConfigurations.push_back(std::move(newConfig));
	fallbackFace.m_LayerConfigurations.push_back(newConfigIdx); // register with the face in case it's asset changes

	return newConfigIdx;
}

//------------------------------------------------
// FontEngine::ConvertLayerForNewFace
//
// convert font face dependent TextLayer data for a new font face
//  - this works based on the internal thresholds being normalized around 0
//
void FontEngine::ConvertLayerForNewFace(TextLayer& layer, float const prevMultiplier, float const newMultiplier)
{
	layer.m_MinThreshold = (layer.m_MinThreshold / prevMultiplier) * newMultiplier;
	layer.m_SdfThreshold = (layer.m_SdfThreshold / prevMultiplier) * newMultiplier;
}

//------------------------------------------------
// FontEngine::HasExistingLayerConfig
//
// return true if there already is a present layer configuration matching the passed one
//
bool FontEngine::HasExistingLayerConfig(LayerConfiguration const& layerConfig, size_t& outConfigIdx)
{
	// check if we already have a set of layers matching this configuration, and return a handle of that
	auto const foundIt = std::find_if(m_LayerConfigurations.begin(), m_LayerConfigurations.end(),
		[layerConfig](LayerConfiguration const& lh)
		{
			return (lh.m_Hash == layerConfig.m_Hash) && (lh.m_FaceIndex == layerConfig.m_FaceIndex) && (lh.m_MainLayerIdx == layerConfig.m_MainLayerIdx);
		});
	if (foundIt != m_LayerConfigurations.end())
	{
		outConfigIdx = static_cast<size_t>(foundIt - m_LayerConfigurations.begin());
		return true;
	}

	return false;
}


} // namespace gui
} // namespace et
