#pragma once
#include <Engine/Graphics/Material.h>


class CubeMap;
class EnvironmentMap;

class SkyboxMaterial : public Material
{
public:
	SkyboxMaterial(T_Hash const assetId);
	virtual ~SkyboxMaterial() = default;

	EnvironmentMap const* GetHDRMap() const { return m_EnvironmentMap.get(); }
	float GetRoughness() { return m_Roughness; }
	void SetRoughness(float r) { m_Roughness = r; }

private:
	void LoadTextures() override;
	void AccessShaderAttributes() override {}
	void UploadDerivedVariables() override;

private:
	AssetPtr<EnvironmentMap> m_EnvironmentMap;
	T_Hash m_AssetId;

	float m_Roughness;
};

