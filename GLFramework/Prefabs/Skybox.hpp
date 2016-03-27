#pragma once
#include "../SceneGraph/Entity.hpp"

class ModelComponent;
class SkyboxMaterial;
class CubeMap;

class Skybox : public Entity
{
public:
	Skybox(string assetFile);
	~Skybox();

	CubeMap* GetCubeMap();
	float GetRoughness();
	void SetRoughness(float r);

protected:

	virtual void Initialize();
	virtual void Update();
	virtual void DrawForward();

private:

	SkyboxMaterial* m_pMaterial = nullptr;
	string m_AssetFile;
private:
	// -------------------------
	// Disabling default copy constructor and default 
	// assignment operator.
	// -------------------------
	Skybox(const Skybox& yRef);
	Skybox& operator=(const Skybox& yRef);
};

