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
	// Construct destruct
	//---------------------
	EditableMeshAsset() : EditorAsset<render::MeshData>() {}
	virtual ~EditableMeshAsset() = default;

	// interface
	//-----------
protected:
	bool LoadFromMemory(std::vector<uint8> const& data) override;

	bool GenerateInternal(BuildConfiguration const& buildConfig, std::string const& dbPath) override;
	bool GenerateRequiresLoadData() const override { return true; }

	// utility
	//---------
private:
	render::MeshDataContainer* LoadAssimp(std::vector<uint8> const& data, std::string const& extension);
	render::MeshDataContainer* LoadGLTF(std::vector<uint8> const& data, std::string const& path, std::string const& extension);
};


} // namespace pl
} // namespace et
