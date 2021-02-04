#include "stdafx.h"
#include "GltfImporter.h"


namespace et {
namespace edit {


//===============
// GLTF Importer
//===============


//--------------------------
// GltfImporter::c-tor
//
GltfImporter::GltfImporter()
	: ImporterBase()
{
	m_SupportedExtensions.push_back("gltf");
	m_SupportedExtensions.push_back("glb");
}


} // namespace edit
} // namespace et
