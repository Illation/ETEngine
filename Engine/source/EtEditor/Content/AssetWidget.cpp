#include "stdafx.h"
#include "AssetWidget.h"

#include <pangomm/layout.h>
#include <gtkmm/frame.h>
#include <gtkmm/label.h>

#include <EtCore/FileSystem/FileUtil.h>

#include <EtPipeline/Content/EditorAsset.h>

#include <EtEditor/Util/GtkUtil.h>


namespace et {
namespace edit {


//===============
// Asset Widget
//===============


//---------------------------
// AssetWidget::AssetWidget
//
AssetWidget::AssetWidget(pl::EditorAssetBase* const asset)
	: m_Asset(asset)
{
	ET_ASSERT(m_Asset != nullptr);

	m_Attachment = Gtk::make_managed<Gtk::Box>(Gtk::ORIENTATION_VERTICAL);

	Gtk::Frame* const viewFrame = Gtk::make_managed<Gtk::Frame>();
	viewFrame->set_shadow_type(Gtk::SHADOW_ETCHED_IN);
	viewFrame->set_halign(Gtk::ALIGN_CENTER);

	Gtk::Label* const label = Gtk::make_managed<Gtk::Label>(core::FileUtil::RemoveExtension(asset->GetAsset()->GetName()).c_str());
	label->set_margin_top(5);
	label->set_margin_bottom(5);

	m_Attachment->pack_start(*viewFrame, false, true, 0);
	m_Attachment->pack_start(*label, false, true, 0);

	if (m_HasPreview)
	{
		// show image
	}
	else
	{
		Pango::AttrList labelAttributes;
		labelAttributes.insert(Pango::Attribute::create_attr_scale(2.5));
		labelAttributes.insert(Pango::Attribute::create_attr_weight(Pango::WEIGHT_BOLD));
		labelAttributes.insert(Pango::Attribute::create_attr_foreground_alpha(50000));

		Gtk::Label* const viewLabel = Gtk::make_managed<Gtk::Label>(core::FileUtil::ExtractExtension(asset->GetAsset()->GetName()).c_str());
		viewLabel->set_attributes(labelAttributes);
		viewLabel->set_margin_top(20);
		viewLabel->set_margin_bottom(20);
		viewLabel->set_margin_left(5);
		viewLabel->set_margin_right(5);

		viewFrame->add(*viewLabel);
	}
}


} // namespace edit
} // namespace et

