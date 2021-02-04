#pragma once


namespace et {
namespace edit {


//--------------------------
// ImporterBase
//
// Base class for file importers
//
class ImporterBase
{
	// definitions
	//-------------
	static std::vector<ImporterBase*> s_Importers;

	// static functionality
	//----------------------
public:
	static void RegisterImporter(ImporterBase* const importer);
	static void RegisterImporters();
	static ImporterBase const* GetImporter(std::string const& filePath);
	static void DestroyImporters();

	// construct destruct
	//--------------------
	ImporterBase() = default;
	virtual ~ImporterBase() = default;

	// interface
	//-----------
	virtual rttr::type GetType() const = 0;

	// accessors
	//-----------
	std::vector<std::string> const& GetExensions() const { return m_SupportedExtensions; }


	// Data
	///////

protected:
	std::vector<std::string> m_SupportedExtensions;
};


} // namespace edit
} // namespace et
