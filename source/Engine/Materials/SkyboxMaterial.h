#pragma once
#include <Engine/Graphics/Material.h>


class CubeMap;
class HDRMapas;

class SkyboxMaterial : public Material
{
public:
	SkyboxMaterial(T_Hash const assetId);
	virtual ~SkyboxMaterial();

	EnvironmentMap const* GetHDRMap() const { return m_EnvironmentMap.get(); }
	float GetRoughness() { return m_Roughness; }
	void SetRoughness(float r) { m_Roughness = r; }

private:
	void LoadTextures();
	void AccessShaderAttributes();

	void UploadDerivedVariables();

private:
	AssetPtr<EnvironmentMap> m_EnvironmentMap;
	T_Hash m_AssetId;

	GLuint m_uNumMipMaps;
	GLuint m_uRoughness;
	float m_Roughness;
};

