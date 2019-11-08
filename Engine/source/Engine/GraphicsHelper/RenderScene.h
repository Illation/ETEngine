#pragma once

typedef core::slot_map<mat4>::id_type T_NodeID;

namespace render {


struct Planet
{
	Material* m_Material;
	T_NodeID m_Transform;
	::Planet m_Planet;
};

struct RenderCollection
{
	struct MeshInstance
	{
		T_NodeID m_Transform;
		Sphere m_BoundingVolume;
	};

	struct Mesh
	{
		T_ArrayLoc m_VAO;
		core::slot_map<MeshInstance> m_Instances;
	};

	struct MaterialInstance
	{
		Material* m_Material;
		core::slot_map<Mesh> m_Meshes;
	};

	AssetPtr<ShaderData> m_Shader;
	core::slot_map<MaterialInstance> m_Materials;
};

struct PointLight
{
	vec3 m_Color;
	float m_Brightness;
	vec3 m_Position;
	float m_Radius;
};
struct DirectionalLight
{
	vec3 m_Color;
	float m_Brightness;
	vec3 m_Direction;
};
struct ShadedDirectionalLight
{
	vec3 m_Color;
	float m_Brightness;
	vec3 m_Direction;
	DirectionalShadowData m_ShadowData;
};

struct Skybox
{
	T_ArrayLoc m_VAO;
	AssetPtr<ShaderData> m_Shader;
	float m_Roughness;
};

struct Atmosphere
{
	T_NodeID m_Transform;
	::Atmosphere* m_Atmosphere;
};

struct Sprite
{
	T_NodeID m_Transform;
	SpriteComponent* m_Sprite;
};


} // namespace render

class RenderScene
{
	core::slot_map<mat4> m_Nodes;

	core::slot_map<render::Planet> m_Terrains;
	core::slot_map<render::RenderCollection> m_OpaqueRenderables;

	core::slot_map<render::PointLight> m_PointLights;
	core::slot_map<render::DirectionalLight> m_DirectionalLights;
	core::slot_map<render::ShadedDirectionalLight> m_ShadedDirectional;

	Skybox m_Skybox;
	core::slot_map<render::RenderCollection> m_ForwardRenderables;
	core::slot_map<render::Atmosphere> m_Atmospheres;

	core::slot_map<render::Sprite> m_Sprites;
};

