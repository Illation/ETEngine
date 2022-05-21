#pragma once
#include <EtGUI/Content/FreetypeFont.h>

#include <EtPipeline/Content/EditorAsset.h>


namespace et {
namespace pl {


//---------------------------------
// EditableFreetypeFontAsset
//
class EditableFreetypeFontAsset final : public EditorAsset<gui::FreetypeFont>
{
	DECLARE_FORCED_LINKING()
	RTTR_ENABLE(EditorAsset<gui::FreetypeFont>)
public:
	// Construct destruct
	//---------------------
	EditableFreetypeFontAsset() : EditorAsset<gui::FreetypeFont>() {}
	virtual ~EditableFreetypeFontAsset() = default;
};


} // namespace pl
} // namespace et
