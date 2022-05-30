#include "stdafx.h"
#include "RmlFontEngineInterface.h"

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
void RmlFontEngineInterface::FontFace::SetAsset(AssetPtr<SdfFont> const asset, Rml::Texture const texture)
{
	m_Font = asset;
	m_Texture = texture;
	m_Version++;

	m_Multiplier = static_cast<float>(m_Size) / static_cast<float>(m_Font->GetFontSize());

	SdfFont::Metric const* const xMetric = m_Font->GetMetric(static_cast<char32>('x'));
	ET_ASSERT(xMetric != nullptr);
	m_XHeight = static_cast<int32>(m_Multiplier * static_cast<float>(xMetric->m_Height));

	m_LineHeight = static_cast<int32>(m_Multiplier * static_cast<float>(m_Font->GetLineHeight()));
	m_Baseline = static_cast<int32>(m_Multiplier * static_cast<float>(m_Font->GetBaseline()));
	m_Underline = static_cast<int32>(m_Multiplier * static_cast<float>(m_Font->GetUnderline()));
	m_UnderlineThickness = static_cast<int32>(m_Multiplier * static_cast<float>(m_Font->GetUnderlineThickness()));
}


//============================
// Font Engine :: Font Family
//============================


//--------------------------------------------------
// RmlFontEngineInterface::FontFamily::GetBestAsset
//
AssetPtr<SdfFont> RmlFontEngineInterface::FontFamily::GetBestAsset(FontFace const& face, Rml::Texture& outTexture) const
{
	AssetPtr<SdfFont> bestMatch;
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
			bestMatch = asset;
			outTexture = m_AssetTextures[assetIdx];
		}
	}

	ET_ASSERT(bestMatch != nullptr);
	return bestMatch;
}


//===========================
// RML Font Engine Interface
//===========================


// static
Rml::FontEffectsHandle const RmlFontEngineInterface::s_InvalidEffects = 0u;
Rml::FontFaceHandle const RmlFontEngineInterface::s_InvalidFont = 0u;


//--------------------------------------
// RmlFontEngineInterface::LoadFontFace
//
bool RmlFontEngineInterface::LoadFontFace(Rml::String const& fileName, bool const fallbackFace, Rml::Style::FontWeight const weight)
{
	UNUSED(fallbackFace); // will need to be handled internally
	UNUSED(weight); // can entirely be ignored since the weights are already generated in the assest

	core::HashString const assetId(fileName.c_str());

	AssetPtr<SdfFont> const font = core::ResourceManager::Instance()->GetAssetData<SdfFont>(assetId);
	if (font == nullptr)
	{
		return false;
	}

	FontFamily& family = FindOrCreateFamily(font->GetFamily());
	if (std::find(family.m_UniqueAssets.cbegin(), family.m_UniqueAssets.cend(), font) != family.m_UniqueAssets.cend())
	{
		return true;
	}

	Rml::Texture texture;
	texture.Set(fileName); // render interface will grab the texture from the font asset - this also allows us to know to use the text shader
	family.m_UniqueAssets.push_back(std::move(font));
	family.m_AssetTextures.push_back(texture);

	for (size_t const faceIdx : family.m_FaceIndices)
	{
		ET_ASSERT(faceIdx < m_Faces.size());
		FontFace& face = m_Faces[faceIdx];

		Rml::Texture bestTex;
		AssetPtr<SdfFont> const bestAsset = family.GetBestAsset(face, bestTex);
		face.SetAsset(bestAsset, bestTex);
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

	auto const foundFaceIt = std::find_if(m_Faces.cbegin(), m_Faces.cend(), [&face](FontFace const& lh) // we assume each handle is only retrieved once
		{
			return (lh.m_Hash != face.m_Hash);
		});

	if (foundFaceIt != m_Faces.cend())
	{
		return static_cast<Rml::FontFaceHandle>(foundFaceIt - m_Faces.cbegin()) + 1;
	}

	size_t const faceIdx = m_Faces.size();

	// find family
	T_FontFamilies::iterator const foundFamilyIt = m_Families.find(face.m_FamilyId);
	if (foundFamilyIt == m_Families.cend())
	{
		ET_ASSERT(false, "Family '%s' couldn't be found, failed to create handle", familyName.c_str());
		return s_InvalidFont;
	}

	FontFamily& family = foundFamilyIt->second;
	family.m_FaceIndices.push_back(faceIdx);

	Rml::Texture bestTex;
	AssetPtr<SdfFont> const bestAsset = family.GetBestAsset(face, bestTex);
	face.SetAsset(bestAsset, bestTex);

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
	UNUSED(fontEffects);

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
int32 RmlFontEngineInterface::GetStringWidth(Rml::FontFaceHandle const faceHandle, Rml::String const& string, Rml::Character const priorCharacter)
{
	FontFace const& face = GetFace(faceHandle);

	float ret = 0.f;

	char32 prevChar = static_cast<char32>(priorCharacter);
	for (char32 const charId : string)
	{
		SdfFont::Metric const* metric = face.m_Font->GetMetric(charId);
		if (metric == nullptr)
		{
			// here we should go to fallback font
			LOG(FS("FontEngine::GetStringWidth > Font '%s' doesn't support char %c", face.m_Font.GetId().ToStringDbg(), charId), core::LogLevel::Warning);
			metric = face.m_Font->GetMetric(' ');
			ET_ASSERT(metric != nullptr);
		}

		float kerning = 0.f;
		if (face.m_Font->UseKerning())
		{
			kerning = metric->GetKerningVec(prevChar).x;
		}

		prevChar = charId;

		ret += (metric->m_AdvanceX + kerning) * face.m_Multiplier;
	}

	return static_cast<int32>(ret);
}

//----------------------------------------
// RmlFontEngineInterface::GenerateString
//
int32 RmlFontEngineInterface::GenerateString(Rml::FontFaceHandle const faceHandle, 
	Rml::FontEffectsHandle const effectsHandle, 
	Rml::String const& string, 
	Rml::Vector2f const& position, 
	Rml::Colourb const& colour, 
	float const opacity, 
	Rml::GeometryList& outGeometry)
{
	UNUSED(effectsHandle);

	FontFace const& face = GetFace(faceHandle);

	outGeometry = Rml::GeometryList();

	// set up geometry
	//-----------------

	outGeometry.push_back(Rml::Geometry());
	Rml::Geometry& geometry = outGeometry.back();

	geometry.SetTexture(&face.m_Texture);

	vec2 const texDim = math::vecCast<float>(face.m_Font->GetAtlas()->GetResolution());

	std::vector<Rml::Vertex>& vertices = geometry.GetVertices();
	std::vector<int32>& indices = geometry.GetIndices();

	indices.reserve(string.size() * 6);

	// Since RmlUi defines a preset vertex layout we're going to have to do some memory magic to also pass the channel indices to the render interface
	size_t const vCountBase = string.size() * 4; // doubles as byte size of channel indices because they are 1 byte each
	size_t const channelVCount = (vCountBase + sizeof(Rml::Vertex) - 1) / sizeof(Rml::Vertex);

	vertices.resize(vCountBase + channelVCount); // append enough vertices to provide a block of memory that we can repurpose for filling in the channels

	uint8* const channels = reinterpret_cast<uint8*>(&vertices[vCountBase]);

	// fill in geometry
	//------------------

	vec2 const pos = RmlUtil::ToEtm(position);

	size_t vIndex = 0u;

	float stringWidth = 0.f; // ret
	char32 prevChar = 0;
	for (char32 const charId : string)
	{
		SdfFont::Metric const* metric = face.m_Font->GetMetric(charId);
		if (metric == nullptr)
		{
			// here we should go to fallback font
			LOG(FS("FontEngine::GenerateString > Font '%s' doesn't support char %c", face.m_Font.GetId().ToStringDbg(), charId), core::LogLevel::Warning);
			metric = face.m_Font->GetMetric(' ');
			ET_ASSERT(metric != nullptr);
		}

		// basic positioning
		vec2 kerning;
		if (face.m_Font->UseKerning())
		{
			kerning = metric->GetKerningVec(prevChar);
		}

		prevChar = charId;

		stringWidth += kerning.x * face.m_Multiplier;

		// indices, counter clockwise winding
		indices.push_back(static_cast<int32>(vIndex));
		indices.push_back(static_cast<int32>(vIndex + 2));
		indices.push_back(static_cast<int32>(vIndex + 1));
		indices.push_back(static_cast<int32>(vIndex + 1));
		indices.push_back(static_cast<int32>(vIndex + 2));
		indices.push_back(static_cast<int32>(vIndex + 3));

		// vertices
		vec2 const charPos(pos.x + (stringWidth + metric->m_OffsetX) * face.m_Multiplier, pos.y + (kerning.y + metric->m_OffsetY) * face.m_Multiplier);
		vec2 const charDim(static_cast<float>(metric->m_Width), static_cast<float>(metric->m_Height));
		vec2 const charDimScaled = charDim * face.m_Multiplier;
		vec2 const charDimTexture = charDim / texDim;
		Rml::Colourb col = (colour.red, colour.green, colour.blue, static_cast<Rml::byte>(static_cast<float>(colour.alpha) * opacity));

		channels[vIndex] = metric->m_Channel;
		Rml::Vertex& v1 = vertices[vIndex++];
		channels[vIndex] = metric->m_Channel;
		Rml::Vertex& v2 = vertices[vIndex++];
		channels[vIndex] = metric->m_Channel;
		Rml::Vertex& v3 = vertices[vIndex++];
		channels[vIndex] = metric->m_Channel;
		Rml::Vertex& v4 = vertices[vIndex++];

		v1.position = Rml::Vector2f(charPos.x, charPos.y + charDimScaled.y);
		v1.colour = col;
		v1.tex_coord = Rml::Vector2f(metric->m_TexCoord.x, metric->m_TexCoord.y + charDimTexture.y);

		v2.position = Rml::Vector2f(charPos.x + charDimScaled.x, charPos.y + charDimScaled.y);
		v2.colour = col;
		v2.tex_coord = Rml::Vector2f(metric->m_TexCoord.x + charDimTexture.x, metric->m_TexCoord.y + charDimTexture.y);

		v3.position = Rml::Vector2f(charPos.x, charPos.y);
		v3.colour = col;
		v3.tex_coord = Rml::Vector2f(metric->m_TexCoord.x, metric->m_TexCoord.y);

		v4.position = Rml::Vector2f(charPos.x + charDimScaled.x, charPos.y);
		v4.colour = col;
		v4.tex_coord = Rml::Vector2f(metric->m_TexCoord.x + charDimTexture.x, metric->m_TexCoord.y);

		// advance
		stringWidth += metric->m_AdvanceX * face.m_Multiplier;
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
RmlFontEngineInterface::FontFamily& RmlFontEngineInterface::FindOrCreateFamily(std::string const& familyName)
{
	core::HashString const familyId(familyName.c_str());

	T_FontFamilies::iterator const foundIt = m_Families.find(familyId);
	if (foundIt != m_Families.cend())
	{
		return foundIt->second;
	}

	auto const res = m_Families.emplace(familyId, familyName);
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


} // namespace gui
} // namespace et
