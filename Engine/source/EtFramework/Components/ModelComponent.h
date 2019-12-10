#pragma once
#include "AbstractComponent.h"

#include <EtCore/Content/AssetPointer.h>
#include <EtCore/Containers/slot_map.h>


class MeshData;


//---------------------------------
// ModelComponent
//
// Component that can draw mesh material combinations
//
class ModelComponent : public AbstractComponent
{
	// construct destruct
	//--------------------
public:
	ModelComponent(T_Hash const meshId, T_Hash const materialId);
	virtual ~ModelComponent() = default;

private:
	ModelComponent(const ModelComponent& yRef);
	ModelComponent& operator=(const ModelComponent& yRef);

	// accessors
	//-----------
public:
	AssetPtr<MeshData> GetMesh() const { return m_Mesh; }

	// component interface
	//---------------------
protected:
	virtual void Init() override;
	virtual void Deinit() override;
	virtual void Update() {}

	// Data
	///////
private:

	T_Hash m_MeshId = 0u;
	AssetPtr<MeshData> m_Mesh;

	T_Hash m_MaterialId = 0u;
	I_AssetPtr m_Material;

	core::T_SlotId m_InstanceId;
};

