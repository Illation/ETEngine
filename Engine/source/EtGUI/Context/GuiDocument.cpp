#include "stdafx.h"
#include "GuiDocument.h"

#include <RmlUi/Core/Core.h>

#include <EtCore/Content/AssetRegistration.h>

#include <EtGUI/Fonts/SdfFont.h>


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
		.property("data model", &GuiDocumentAsset::m_DataModelId)
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

	for (core::I_Asset::Reference const& reference : GetReferences())
	{
		I_AssetPtr const* const rawAssetPtr = reference.GetAsset();

		if (rawAssetPtr->GetType() == rttr::type::get<SdfFont>())
		{
			SdfFontAsset const* const fontAsset = static_cast<SdfFontAsset const*>(rawAssetPtr->GetAsset());
			Rml::LoadFontFace(fontAsset->GetPath() + fontAsset->GetName(), fontAsset->m_IsFallbackFont);
		}
	}

	// all done
	return true;
}


} // namespace gui
} // namespace et

