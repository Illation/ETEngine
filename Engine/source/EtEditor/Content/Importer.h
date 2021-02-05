#pragma once


namespace et {
namespace edit {


//--------------------------
// E_ImportResult
//
enum class E_ImportResult : int8
{
	Cancelled = -1,
	Failed = 0,
	Succeeded = 1
};


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
	static std::vector<std::string const*> GetAllSupportedExtensions();

	// construct destruct
	//--------------------
	ImporterBase() = default;
	virtual ~ImporterBase() = default;

	// interface
	//-----------
	virtual rttr::type GetType() const = 0;
	virtual char const* GetTitle() const = 0;

	// accessors
	//-----------
	std::vector<std::string> const& GetExensions() const { return m_SupportedExtensions; }

	// functionality
	//---------------
	E_ImportResult Run(std::string const& filePath) const;


	// Data
	///////

protected:
	std::vector<std::string> m_SupportedExtensions;
};


} // namespace edit
} // namespace et
