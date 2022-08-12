#pragma once
#include <EtGUI/Context/GuiDocument.h>

#include <EtPipeline/Core/Content/EditorAsset.h>


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
