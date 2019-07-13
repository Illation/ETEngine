#pragma once
#include <Engine/SceneGraph/Entity.h>

class ModelComponent;
class SkyboxMaterial;
class CubeMap;

class Skybox : public Entity
{
public:
	Skybox(std::string assetFile);
	~Skybox();

	//CubeMap* GetCubeMap();
	HDRMap* GetHDRMap();
	float GetRoughness();
	void SetRoughness(float r);

protected:

	virtual void Initialize();
	virtual void Update();
	virtual void DrawForward();

private:

	SkyboxMaterial* m_pMaterial = nullptr;
	std::string m_AssetFile;
private:
	// -------------------------
	// Disabling default copy constructor and default 
	// assignment operator.
	// -------------------------
	Skybox(const Skybox& yRef);
	Skybox& operator=(const Skybox& yRef);
};

