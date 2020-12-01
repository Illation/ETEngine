#pragma once
#include <EtCore/Content/AssetPointer.h>
#include <EtCore/Containers/slot_map.h>

#include <EtRendering/GraphicsTypes/Mesh.h>
#include <EtRendering/MaterialSystem/MaterialInterface.h>

#include <EtFramework/SceneGraph/ComponentDescriptor.h>


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
	ModelComponent(AssetPtr<render::MeshData> const mesh, AssetPtr<render::I_Material> const material);
	~ModelComponent() = default;

	// accessors
	//-----------
	AssetPtr<render::MeshData> GetMesh() const { return m_Mesh; }

	// Data
	///////
private:

	AssetPtr<render::MeshData> m_Mesh;
	AssetPtr<render::I_Material> m_Material;

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

	AssetPtr<render::MeshData> mesh;
	AssetPtr<render::I_Material> material;
};


} // namespace fw
} // namespace et
