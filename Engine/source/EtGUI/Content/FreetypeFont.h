#pragma once
#include <EtCore/Content/Asset.h>
#include <EtCore/Util/LinkerUtils.h>


namespace et {
	REGISTRATION_NS(gui);
}


namespace et {
namespace gui {


//---------------------------------
// FreetypeFont
//
// Loads a dynamic (freetype based) font into Rml on construction.
//  - Asset can be dereferenced immediately after since the font data will already be stored by the RML default font engine interface
//  - #todo: remove this once we replace RMLs font engine interface with a custom solution
//
class FreetypeFont final
{
	friend class FreetypeFontAsset;
	REGISTRATION_FRIEND_NS(gui)

	FreetypeFont() = default;
	FreetypeFont(std::vector<uint8> const& data, bool const isFallback);
public:
	~FreetypeFont() = default;
};


//---------------------------------
// FreetypeFontAsset
//
// Font asset designed such that simply including it as a reference in another asset ensures that it is loaded
//
class FreetypeFontAsset final : public core::Asset<FreetypeFont, false>
{
	DECLARE_FORCED_LINKING()
	RTTR_ENABLE(core::Asset<FreetypeFont, false>)

	struct LoadedFontData
	{
		LoadedFontData() = default;
		LoadedFontData(core::HashString const id, std::vector<uint8> const& data) : m_Id(id), m_Data(data) {}

		core::HashString m_Id;
		std::vector<uint8> m_Data; // RmlUi requires the font data to stay in memory until after the library is shut down
	};

	static std::vector<LoadedFontData> s_LoadedFonts;

public:
	// Construct destruct
	//---------------------
	FreetypeFontAsset() : core::Asset<FreetypeFont, false>() {}
	virtual ~FreetypeFontAsset() = default;

	// Asset overrides
	//---------------------
	bool LoadFromMemory(std::vector<uint8> const& data) override;

	// Data
	///////
public:
	bool m_IsFallbackFont = false;
};


} // namespace gui
} // namespace et
