#pragma once
#include <EtCore/Content/AssetPointer.h>
#include <EtCore/Containers/slot_map.h>

#include <EtFramework/SceneGraph/ComponentDescriptor.h>


// fwd
namespace et { namespace render { 
	class MeshData;
} }


namespace et {
namespace fw {


//---------------------------------
// ModelComponent
//
// Component that can draw mesh material combinations
//
class ModelComponent final
{
	// definitions
	//-------------
	ECS_DECLARE_COMPONENT

	friend class ModelInit;

	// construct destruct
	//--------------------
public:
	ModelComponent(core::HashString const meshId, core::HashString const materialId);
	ModelComponent(AssetPtr<render::MeshData> const mesh, I_AssetPtr const material);
	~ModelComponent() = default;

	// accessors
	//-----------
	AssetPtr<render::MeshData> GetMesh() const { return m_Mesh; }

	// Data
	///////
private:

	AssetPtr<render::MeshData> m_Mesh;
	I_AssetPtr m_Material;

	core::T_SlotId m_InstanceId = core::INVALID_SLOT_ID;
};


//---------------------------------
// ModelComponentDesc
//
// Descriptor for serialization and deserialization of model components
//
class ModelComponentDesc final : public ComponentDescriptor<ModelComponent>
{
	// definitions
	//-------------
	RTTR_ENABLE(ComponentDescriptor<ModelComponent>)

	// construct destruct
	//--------------------
public:
	ModelComponentDesc() : ComponentDescriptor<ModelComponent>() {}
	~ModelComponentDesc() = default;

	// ComponentDescriptor interface
	//-------------------------------
	ModelComponent* MakeData() override;

	// Data
	///////

	core::HashString mesh;
	core::HashString material;
};


} // namespace fw
} // namespace et
