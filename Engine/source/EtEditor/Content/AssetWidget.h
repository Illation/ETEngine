#pragma once

#include <gtkmm/box.h>


namespace et { namespace core {
	class I_Asset;
} }


namespace et {
namespace edit {


//---------------
// AssetWidget
//
// Display a thumbnail of an asset
//
class AssetWidget final
{
public:
	AssetWidget(core::I_Asset* const asset);

	Gtk::Widget* GetAttachment() const { return m_Attachment; }
	core::I_Asset* GetAsset() const { return m_Asset; }

	// Data
	///////

private:
	core::I_Asset* m_Asset = nullptr; // not using assetptr since we don't want the widget to enforce loading the asset

	Gtk::Box* m_Attachment = nullptr;
	bool m_HasPreview = false;
};


} // namespace edit
} // namespace et

