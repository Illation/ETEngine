#pragma once
#include <EtCore/Content/AssetPointer.h>
#include <EtCore/Containers/slot_map.h>

#include <EtRendering/GraphicsTypes/Mesh.h>
#include <EtRendering/MaterialSystem/MaterialInterface.h>

#include <EtFramework/SceneGraph/ComponentDescriptor.h>


namespace et {
	REGISTRATION_NS(fw);
}


namespace et {
namespace fw {


//---------------------------------
// ModelComponent
//
// Component that can draw mesh material combinations
//
class ModelComponent final : public SimpleComponentDescriptor
{
	// definitions
	//-------------
	ECS_DECLARE_COMPONENT

	RTTR_ENABLE(SimpleComponentDescriptor) // for serialization
	REGISTRATION_FRIEND_NS(fw)

	friend class ModelInit;

	// construct destruct
	//--------------------
public:
	ModelComponent() = default;
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


} // namespace fw
} // namespace et
