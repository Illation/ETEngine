#pragma once
#include <EtRendering/GraphicsTypes/Mesh.h>

#include <EtPipeline/Content/EditorAsset.h>


namespace et {
namespace pl {
	

//---------------------------------
// EditableMeshAsset
//
class EditableMeshAsset final : public EditorAsset<render::MeshData>
{
	DECLARE_FORCED_LINKING()
	RTTR_ENABLE(EditorAsset<render::MeshData>)
public:
	static std::string const s_EtMeshExt;

	// Construct destruct
	//---------------------
	EditableMeshAsset() : EditorAsset<render::MeshData>() {}
	virtual ~EditableMeshAsset() = default;
};


} // namespace pl
} // namespace et
