#pragma once
#include "Importer.h"


namespace et {
namespace edit {


//--------------------------
// GltfImporter
//
// Import assets from gltf files
//
class GltfImporter : public ImporterBase
{
	// construct destruct
	//--------------------
public:
	GltfImporter();
	~GltfImporter() = default;

	// importer interface
	//--------------------
	rttr::type GetType() const override { return rttr::type::get(*this); }
	char const* GetTitle() const override {
		return "GLTF Importer";
	}
};


} // namespace edit
} // namespace et
