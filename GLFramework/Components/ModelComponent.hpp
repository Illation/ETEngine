#pragma once
#include "AbstractComponent.hpp"
#include <string>

class Material;
class MeshFilter;
class ModelComponent : public AbstractComponent
{
public:
	ModelComponent(std::string assetFile);
	~ModelComponent();

	void SetMaterial(Material* pMat);

protected:

	virtual void Initialize();
	virtual void Update();
	virtual void Draw();

private:

	void UpdateMaterial();

	std::string m_AssetFile;
	MeshFilter* m_pMeshFilter = nullptr;
	Material* m_pMaterial = nullptr;
	bool m_MaterialSet = false;

private:
	// -------------------------
	// Disabling default copy constructor and default 
	// assignment operator.
	// -------------------------
	ModelComponent(const ModelComponent& yRef);
	ModelComponent& operator=(const ModelComponent& yRef);
};

