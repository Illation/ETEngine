#pragma once
#include <EtRendering/MaterialSystem/MaterialData.h>
#include <EtRendering/MaterialSystem/MaterialInstance.h>

#include <EtPipeline/Content/EditorAsset.h>


namespace et {
namespace pl {
	

//---------------------------------
// EditableMaterialAsset
//
class EditableMaterialAsset final : public EditorAsset<render::Material>
{
	DECLARE_FORCED_LINKING()
	RTTR_ENABLE(EditorAsset<render::Material>)
public:
	// Construct destruct
	//---------------------
	EditableMaterialAsset() : EditorAsset<render::Material>() {}
	virtual ~EditableMaterialAsset() = default;
};


//---------------------------------
// EditableMaterialInstanceAsset
//
class EditableMaterialInstanceAsset final : public EditorAsset<render::MaterialInstance>
{
	RTTR_ENABLE(EditorAsset<render::MaterialInstance>)
public:
	// Construct destruct
	//---------------------
	EditableMaterialInstanceAsset() : EditorAsset<render::MaterialInstance>() {}
	virtual ~EditableMaterialInstanceAsset() = default;
};


} // namespace pl
} // namespace et
