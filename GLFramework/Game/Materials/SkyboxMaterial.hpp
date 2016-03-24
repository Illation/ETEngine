#pragma once
#include "../../Graphics/Material.hpp"

class CubeMap;

class SkyboxMaterial : public Material
{
public:
	SkyboxMaterial(string assetFile);
	~SkyboxMaterial();

	CubeMap* GetCubeMap() { return m_pMap; }

private:
	void LoadTextures();
	void AccessShaderAttributes();

	void UploadDerivedVariables();

private:
	CubeMap *m_pMap = nullptr;
	string m_AssetFile;
};

