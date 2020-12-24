#pragma once

#include <gtkmm/box.h>


namespace et { namespace pl {
	class EditorAssetBase;
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
	AssetWidget(pl::EditorAssetBase* const asset);

	Gtk::Widget* GetAttachment() const { return m_Attachment; }
	pl::EditorAssetBase* GetAsset() const { return m_Asset; }

	// Data
	///////

private:
	pl::EditorAssetBase* m_Asset = nullptr; // not using assetptr since we don't want the widget to enforce loading the asset

	Gtk::Box* m_Attachment = nullptr;
	bool m_HasPreview = false;
};


} // namespace edit
} // namespace et

