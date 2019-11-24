#pragma once
#include "AbstractComponent.h"

#include <EtCore/Content/AssetPointer.h>
#include <EtCore/Containers/slot_map.h>


class Material;
class MeshData;


//---------------------------------
// ModelComponent
//
// Component that can draw mesh material combinations
//
class ModelComponent : public AbstractComponent
{
	// definitions
	//-------------
public:
	enum class CullMode
	{
		SPHERE,
		DISABLED
	};

	// construct destruct
	//--------------------
	ModelComponent(T_Hash const assetId);
	virtual ~ModelComponent() = default;

private:
	ModelComponent(const ModelComponent& yRef);
	ModelComponent& operator=(const ModelComponent& yRef);

	// functionality
	//---------------
public:
	void SetMaterial(Material* pMat);
	void SetCullMode(CullMode mode) { m_CullMode = mode; }

	// accessors
	//-----------
	AssetPtr<MeshData> GetMesh() const { return m_Mesh; }

	// component interface
	//---------------------
protected:
	virtual void Init() override;
	virtual void Deinit() override;
	virtual void Update() override;

	// utility
	//---------
private:
	void UpdateMaterial();

	// Data
	///////

	T_Hash m_AssetId;
	AssetPtr<MeshData> m_Mesh;

	Material* m_Material = nullptr;
	bool m_MaterialSet = false;

	CullMode m_CullMode = CullMode::SPHERE;

	core::T_SlotId m_InstanceId;
};

