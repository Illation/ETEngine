#pragma once


template <class TComponent>
struct RenderObject;
{
	TComponent* comp;
	TransformComponent const* transform;
}


class RenderScene
{
	std::vector<RenderObject<ModelComponent>> m_Models;
	std::vector<RenderObject<SpriteComponent>> m_Sprites;

	std::vector<RenderObject<Atmosphere>> m_Atmospheres;

	std::vector<RenderObject<LightComponent>> m_Lights;
};

