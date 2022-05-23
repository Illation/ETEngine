#include "stdafx.h"
#include "FreetypeFont.h"

#include <RmlUi/Core/Core.h>

#include <EtCore/Content/AssetRegistration.h>

#include <EtGUI/Context/RmlGlobal.h>


namespace et {
namespace gui {


//===============
// Freetype Font 
//===============


//-----------------------------------
// FreetypeFontAsset::c-tor
//
// Load a freetype font into RML
//
FreetypeFont::FreetypeFont(std::vector<uint8> const& data, bool const isFallback)
{
	ET_ASSERT(RmlGlobal::IsInitialized());

	// #todo: it would be good if we could have a handle to unload fonts from RML again, so that we can controll what is loaded in memory
	Rml::LoadFontFace(reinterpret_cast<Rml::byte const*>(data.data()), 
		static_cast<int32>(data.size()), 
		"", // Rml should automatically deduce the family name
		Rml::Style::FontStyle::Normal, 
		Rml::Style::FontWeight::Auto, 
		isFallback);
}


//=====================
// Freetype Font Asset
//=====================


// reflection
RTTR_REGISTRATION
{
	BEGIN_REGISTER_CLASS_ASSET(FreetypeFont, "freetype font")
	END_REGISTER_CLASS(FreetypeFont);

	BEGIN_REGISTER_CLASS(FreetypeFontAsset, "freetype font asset")
		.property("is fallback", &FreetypeFontAsset::m_IsFallbackFont)
	END_REGISTER_CLASS_POLYMORPHIC(FreetypeFontAsset, core::I_Asset);
}
DEFINE_FORCED_LINKING(FreetypeFontAsset) // force the asset class to be linked as it is only used in reflection


// static
std::vector<core::HashString> FreetypeFontAsset::s_LoadedFonts;


//-----------------------------------
// FreetypeFontAsset::LoadFromMemory
//
// Load a freetype font from memory
//
bool FreetypeFontAsset::LoadFromMemory(std::vector<uint8> const& data)
{
	// Since RML stores fonts persistently we make sure we only use the loading constructor once
	bool const isLoaded = std::find(s_LoadedFonts.cbegin(), s_LoadedFonts.cend(), GetId()) != s_LoadedFonts.cend();
	if (isLoaded)
	{
		m_Data = new FreetypeFont();
	}
	else
	{
		s_LoadedFonts.push_back(GetId());
		m_Data = new FreetypeFont(data, m_IsFallbackFont);
	}

	// all done
	return true;
}


} // namespace gui
} // namespace et
