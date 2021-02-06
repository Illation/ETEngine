#include "stdafx.h"
#include "Importer.h"

#include <gtkmm/button.h>
#include <gtkmm/dialog.h>
#include <gtkmm/label.h>

#include <EtCore/FileSystem/Entry.h>
#include <EtCore/FileSystem/FileUtil.h>

#include "GltfImporter.h"
#include <EtEditor/Util/GtkUtil.h>


namespace et {
namespace edit {


//===============
// Importer Base
//===============


std::vector<ImporterBase*> ImporterBase::s_Importers;


//--------------------------------
// ImporterBase::RegisterImporter
//
void ImporterBase::RegisterImporter(ImporterBase* const importer)
{
	rttr::type const importerType = importer->GetType();
	if (std::find_if(s_Importers.cbegin(), s_Importers.cend(), [importerType](ImporterBase const* const rhs)
		{
			return importerType == rhs->GetType();
		}) == s_Importers.cend())
	{
		s_Importers.push_back(importer);
	}
	else
	{
		ET_ASSERT(false, "importer of type '%s' already registered", importerType.get_name().data());
	}
}

//--------------------------------
// ImporterBase::RegisterImporter
//
void ImporterBase::RegisterImporters()
{
	RegisterImporter(new GltfImporter());
	// others
}

//--------------------------
// ImporterBase::GetImporter
//
ImporterBase* ImporterBase::GetImporter(std::string const& filePath)
{
	std::string const ext = core::FileUtil::ExtractExtension(filePath);
	auto const foundIt = std::find_if(s_Importers.cbegin(), s_Importers.cend(), [&ext](ImporterBase const* const importer)
		{
			std::vector<std::string> const& extensions = importer->GetExensions();
			return (std::find(extensions.cbegin(), extensions.cend(), ext) != extensions.cend());
		});

	if (foundIt != s_Importers.cend())
	{
		return *foundIt;
	}

	return nullptr;
}

//--------------------------
// ImporterBase::DestroyImporters
//
void ImporterBase::DestroyImporters()
{
	for (ImporterBase* const importer : s_Importers)
	{
		delete importer;
	}

	s_Importers.clear();
}

//-----------------------------------------
// ImporterBase::GetAllSupportedExtensions
//
std::vector<std::string const*> ImporterBase::GetAllSupportedExtensions()
{
	std::vector<std::string const*> ret;

	for (ImporterBase* const importer : s_Importers)
	{
		for (std::string const& ext : importer->GetExensions())
		{
			ret.push_back(&ext);
		}
	}

	return ret;
}

//-------------------
// ImporterBase::Run
//
E_ImportResult ImporterBase::Run(std::string const& filePath, std::string const& outDirectory, Gtk::Window& parent, E_ImportAll& importAll) 
{
	E_ImportResult result = E_ImportResult::Cancelled;
	bool runImport = false;

	// setup dialog for import options
	//---------------------------------
	if (importAll != E_ImportAll::True) // we can skip import dialog for subsequent assets
	{
		Gtk::Dialog dialog(Glib::ustring(GetTitle()), parent, Gtk::DIALOG_DESTROY_WITH_PARENT | Gtk::DIALOG_MODAL);
		Gtk::Box* const content = dialog.get_content_area();

		// buttons
		Gtk::Button* const importBtn = dialog.add_button("Import", Gtk::ResponseType::RESPONSE_ACCEPT);
		Gtk::Button* importAllBtn = nullptr;
		if (importAll != E_ImportAll::Disabled)
		{
			importAllBtn = dialog.add_button("Import All", Gtk::ResponseType::RESPONSE_ACCEPT);
			importAllBtn->signal_button_release_event().connect([&importAll](GdkEventButton* const)
				{
					importAll = E_ImportAll::True;
					return false;
				});
		}

		dialog.add_button("Cancel", Gtk::ResponseType::RESPONSE_CANCEL);

		// general tool UI
		Gtk::Label* const fileLabel = Gtk::make_managed<Gtk::Label>(Glib::ustring(core::FileUtil::ExtractName(filePath)));
		fileLabel->set_tooltip_text(filePath);
		content->pack_start(*fileLabel, false, true, 3u);

		// custom tool options
		if (HasOptions())
		{
			Gtk::Frame* const optionFrame = Gtk::make_managed<Gtk::Frame>();
			optionFrame->set_shadow_type(Gtk::SHADOW_ETCHED_IN);
			optionFrame->set_label("Options");
			content->pack_start(*optionFrame, true, true, 2u);

			SetupOptions(optionFrame, T_SensitiveFn([importBtn, importAllBtn](bool const isSensitive)
				{
					importBtn->set_sensitive(isSensitive);
					if (importAllBtn != nullptr)
					{
						importAllBtn->set_sensitive(isSensitive);
					}
				}));
		}

		// check if we should actually import
		content->show_all_children();
		dialog.set_resizable(false);

		int32 const response = dialog.run();
		if (response == Gtk::ResponseType::RESPONSE_ACCEPT)
		{
			runImport = true;
		}

		dialog.hide();
	}
	else
	{
		runImport = true;
	}

	// run importer
	//--------------
	if (runImport)
	{
		std::vector<uint8> importData;
		{
			core::File importFile(filePath, nullptr);
			if (!importFile.Open(core::FILE_ACCESS_MODE::Read))
			{
				ET_ASSERT(false, "failed to open import file '%s'", filePath.c_str());
				return E_ImportResult::Failed;
			}

			importData = importFile.Read();
		}

		if (Import(importData, filePath, outDirectory))
		{
			result = E_ImportResult::Succeeded;
		}
		else
		{
			result = E_ImportResult::Failed;
		}
	}

	return result;
}


} // namespace edit
} // namespace et
