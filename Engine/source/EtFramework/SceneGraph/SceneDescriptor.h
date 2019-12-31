#pragma once
#include "ComponentDescriptor.h"
#include "EntityLink.h"


namespace framework {


class EntityDescriptor
{
private:
	T_EntityId m_Id;
	std::vector<I_ComponentDescriptor*> m_Components;
	std::vector<EntityDescriptor*> m_Children;
};


struct SceneDescriptor
{
	// entities
	std::vector<EntityDescriptor> entities;

	// graphical parameters
	std::string skybox;
	std::string starfield;
	EntityLink activeCamera;
	PostProcessingSettings postprocessing;

	// audio parameters
	EntityLink audioListener;

	// physics parameters
	vec3 gravity;
};


} // namespace framework

