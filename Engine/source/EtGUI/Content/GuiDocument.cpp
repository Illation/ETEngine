#include "stdafx.h"
#include "GuiDocument.h"

#include <EtCore/Content/AssetRegistration.h>


namespace et {
namespace gui {


//====================
// GUI Document Asset
//====================


// reflection
RTTR_REGISTRATION
{
	BEGIN_REGISTER_CLASS_ASSET(GuiDocument, "gui document")
	END_REGISTER_CLASS(GuiDocument);

	BEGIN_REGISTER_CLASS(GuiDocumentAsset, "gui document asset")
	END_REGISTER_CLASS_POLYMORPHIC(GuiDocumentAsset, core::I_Asset);
}
DEFINE_FORCED_LINKING(GuiDocumentAsset) // force the asset class to be linked as it is only used in reflection


//----------------------------------
// GuiDocumentAsset::LoadFromMemory
//
// Load RML data from a file
//
bool GuiDocumentAsset::LoadFromMemory(std::vector<uint8> const& data)
{
	// Create data as a view of loaded memory
	m_Data = new GuiDocument();
	m_Data->m_Text = reinterpret_cast<char const*>(data.data());
	m_Data->m_Length = data.size();

	// all done
	return true;
}


} // namespace gui
} // namespace et

