#include "stdafx.h"
#include "Importer.h"

#include <EtCore/FileSystem/FileUtil.h>

#include "GltfImporter.h"


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
ImporterBase const* ImporterBase::GetImporter(std::string const& filePath)
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


} // namespace edit
} // namespace et
