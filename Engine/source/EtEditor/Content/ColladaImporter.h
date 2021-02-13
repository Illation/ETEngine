#pragma once
#include "Importer.h"


namespace et {
namespace edit {


//--------------------------
// ColladaImporter
//
// Import assets from dae files
//
class ColladaImporter : public ImporterBase
{
	// construct destruct
	//--------------------
public:
	ColladaImporter();
	~ColladaImporter() = default;

	// importer interface
	//--------------------
	rttr::type GetType() const override { return rttr::type::get(*this); }
	char const* GetTitle() const override { return "Collada Importer"; }

	virtual bool HasOptions() const { return true; }
	virtual void SetupOptions(Gtk::Frame* const frame, T_SensitiveFn& sensitiveFn);

	bool Import(std::vector<uint8> const& importData, std::string const& dirPath, std::vector<pl::EditorAssetBase*>& outAssets) const override;

	// Data
	///////

private:
	// asset types
	bool m_ImportMeshes = true;
	bool m_ImportMaterials = false;
	bool m_ImportTextures = false;
	bool m_ImportSkeletons = false;
	bool m_ImportAnimations = false;
	bool m_ImportTemplates = false;

	// mesh options
	bool m_CalculateTangentSpace = true;
	bool m_RemoveDuplicateVertices = false;
	bool m_IncludeSkeletalData = false;
};


} // namespace edit
} // namespace et