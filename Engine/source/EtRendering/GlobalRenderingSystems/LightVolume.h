#pragma once
#include <EtCore/Content/AssetPointer.h>

class ShaderData;
class LightMaterial;
namespace render {
	class DirectionalShadowData;
}

class PointLightVolume final
{
public:
	PointLightVolume() = default;
	~PointLightVolume();

	void Draw(vec3 pos, float radius, vec3 col);

private:
	void Initialize();
	bool IsInitialized = false;

	LightMaterial* m_pMaterial;
};

class DirectLightVolume final
{
public:
	DirectLightVolume() = default;
	~DirectLightVolume() = default;

	void Draw(vec3 dir, vec3 col);
	void DrawShadowed(vec3 dir, vec3 col, render::DirectionalShadowData const& shadow);

protected:
	AssetPtr<ShaderData> m_Shader;
	AssetPtr<ShaderData> m_ShaderShadowed;
private:
	void Initialize();
	bool m_IsInitialized = false;
};