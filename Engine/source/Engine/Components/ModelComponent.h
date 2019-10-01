#pragma once
#include "AbstractComponent.h"

#include <EtCore/Content/AssetPointer.h>


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

	// component interface
	//---------------------
protected:
	virtual void Initialize();
	virtual void Update();
	virtual void Draw();
	virtual void DrawForward();
	virtual void DrawMaterial(Material* const mat);

	// utility
	//---------
private:
	void UpdateMaterial();
	void DrawCall();

	// Data
	///////

	T_Hash m_AssetId;
	AssetPtr<MeshData> m_Mesh;

	Material* m_Material = nullptr;
	bool m_MaterialSet = false;

	CullMode m_CullMode = CullMode::SPHERE;
};

