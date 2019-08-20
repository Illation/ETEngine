#pragma once
#include <Engine/SceneGraph/Entity.h>

class ModelComponent;
class SkyboxMaterial;
class CubeMap;

class Skybox : public Entity
{
public:
	Skybox(T_Hash const assetId);
	virtual ~Skybox();

	EnvironmentMap const* GetHDRMap() const;
	float GetRoughness() const;

	void SetRoughness(float r);

protected:

	virtual void Initialize();
	virtual void Update();
	virtual void DrawForward();

private:

	SkyboxMaterial* m_pMaterial = nullptr;
	T_Hash m_AssetId;
private:
	// -------------------------
	// Disabling default copy constructor and default 
	// assignment operator.
	// -------------------------
	Skybox(const Skybox& yRef);
	Skybox& operator=(const Skybox& yRef);
};

