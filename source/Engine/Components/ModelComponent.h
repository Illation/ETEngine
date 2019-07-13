#pragma once
#include "AbstractComponent.h"


class Material;
class MeshFilter;
class ModelComponent : public AbstractComponent
{
public:
	ModelComponent(std::string assetFile);
	~ModelComponent();

	void SetMaterial(Material* pMat);

	enum class CullMode
	{
		SPHERE,
		DISABLED
	};
	void SetCullMode(CullMode mode) { m_CullMode = mode; }


protected:

	virtual void Initialize();
	virtual void Update();
	virtual void Draw();
	virtual void DrawForward();
	virtual void DrawShadow();

private:

	void UpdateMaterial();
	void DrawCall();

	std::string m_AssetFile;
	MeshFilter* m_pMeshFilter = nullptr;
	Material* m_pMaterial = nullptr;
	bool m_MaterialSet = false;
	CullMode m_CullMode = CullMode::SPHERE;

private:
	// -------------------------
	// Disabling default copy constructor and default 
	// assignment operator.
	// -------------------------
	ModelComponent(const ModelComponent& yRef);
	ModelComponent& operator=(const ModelComponent& yRef);
};

