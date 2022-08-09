#pragma once
#include <EtCore/Content/AssetPointer.h>


namespace et {
namespace render {


class rhi::ShaderData;
class Material;
class DirectionalShadowData;
class Gbuffer;


class PointLightVolume final
{
public:
	PointLightVolume() = default;
	~PointLightVolume() = default;

	void Draw(vec3 pos, float radius, vec3 col, Gbuffer const& gbuffer);

private:
	AssetPtr<render::Material> m_Material;
};

class DirectLightVolume final
{
public:
	DirectLightVolume() = default;
	~DirectLightVolume() = default;

	void Draw(vec3 dir, vec3 col, Gbuffer const& gbuffer);
	void DrawShadowed(vec3 dir, vec3 col, render::DirectionalShadowData const& shadow, Gbuffer const& gbuffer);

protected:
	AssetPtr<rhi::ShaderData> m_Shader;
	AssetPtr<rhi::ShaderData> m_ShaderShadowed;
private:
	void Initialize();
	bool m_IsInitialized = false;
};


} // namespace render
} // namespace et
