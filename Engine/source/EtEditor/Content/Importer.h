#pragma once
#include <gtkmm/frame.h>
#include <gtkmm/window.h>

#include <EtPipeline/Content/EditorAsset.h>


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
// E_ImportAll
//
enum class E_ImportAll : int8
{
	Disabled = -1,
	False = 0,
	True = 1
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

protected:
	typedef std::function<void(bool const)> T_SensitiveFn;

	// static functionality
	//----------------------
public:
	static void RegisterImporter(ImporterBase* const importer);
	static void RegisterImporters();
	static ImporterBase* GetImporter(std::string const& filePath);
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
protected:
	virtual bool HasOptions() const { return false; }
	virtual void SetupOptions(Gtk::Frame* const frame, T_SensitiveFn& sensitiveFn) { UNUSED(frame); UNUSED(sensitiveFn); }
	virtual bool Import(std::vector<uint8> const& importData, std::string const& filePath, std::vector<pl::EditorAssetBase*>& outAssets) const = 0;

	// accessors
	//-----------
public:
	std::vector<std::string> const& GetExensions() const { return m_SupportedExtensions; }

	// functionality
	//---------------
	E_ImportResult Run(std::string const& filePath, std::string const& outDirectory, bool const isProjectDb, Gtk::Window& parent, E_ImportAll& importAll);


	// Data
	///////

protected:
	std::vector<std::string> m_SupportedExtensions;
};


} // namespace edit
} // namespace et
