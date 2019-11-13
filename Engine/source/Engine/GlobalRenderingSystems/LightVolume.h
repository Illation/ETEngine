#pragma once
#include <EtCore/Content/AssetPointer.h>

class ShaderData;
class LightMaterial;
class NullMaterial;
class DirectionalShadowData;

class PointLightVolume final
{
public:
	PointLightVolume();
	~PointLightVolume();

	void Draw(vec3 pos, float radius, vec3 col);

private:
	void Initialize();
	bool IsInitialized = false;

	LightMaterial* m_pMaterial;
	NullMaterial* m_pNullMaterial;
};

class DirectLightVolume final
{
public:
	DirectLightVolume();
	~DirectLightVolume();

	void Draw(vec3 dir, vec3 col);
	void DrawShadowed(vec3 dir, vec3 col, DirectionalShadowData const& shadow);

protected:
	AssetPtr<ShaderData> m_pShader;
	AssetPtr<ShaderData> m_pShaderShadowed;
private:
	void Initialize();
	bool m_IsInitialized = false;
};