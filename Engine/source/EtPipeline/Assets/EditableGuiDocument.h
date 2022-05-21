#pragma once
#include <EtGUI/Content/GuiDocument.h>

#include <EtPipeline/Content/EditorAsset.h>


namespace et {
namespace pl {


//---------------------------------
// EditableGuiDocumentAsset
//
class EditableGuiDocumentAsset final : public EditorAsset<gui::GuiDocument>
{
	DECLARE_FORCED_LINKING()
	RTTR_ENABLE(EditorAsset<gui::GuiDocument>)
public:
	// Construct destruct
	//---------------------
	EditableGuiDocumentAsset() : EditorAsset<gui::GuiDocument>() {}
	virtual ~EditableGuiDocumentAsset() = default;
};


} // namespace pl
} // namespace et
