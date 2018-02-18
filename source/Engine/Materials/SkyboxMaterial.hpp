#pragma once
#include "../Graphics/Material.hpp"

class CubeMap;
class HDRMapas;

class SkyboxMaterial : public Material
{
public:
	SkyboxMaterial(std::string assetFile);
	~SkyboxMaterial();

	CubeMap* GetCubeMap() { return m_pMap; }
	HDRMap* GetHDRMap() { return m_pHDRMap; }
	float GetRoughness() { return m_Roughness; }
	void SetRoughness(float r) { m_Roughness = r; }

private:
	void LoadTextures();
	void AccessShaderAttributes();

	void UploadDerivedVariables();

private:
	CubeMap *m_pMap = nullptr;
	HDRMap *m_pHDRMap = nullptr;
	std::string m_AssetFile;

	GLuint m_uNumMipMaps;
	GLuint m_uRoughness;
	float m_Roughness;
};

