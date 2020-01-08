#pragma once
#include <EtFramework/Components/PlanetComponent.h>

#include <EtFramework/ECS/EcsController.h>


namespace fw {


//-------------
// PlanetInit
//
// Events for adding and removing planet components from the scene
//
class PlanetInit final 
{
public:
	static void OnComponentAdded(EcsController& controller, PlanetComponent& component, T_EntityId const entity);
	static void OnComponentRemoved(EcsController& controller, PlanetComponent& component, T_EntityId const entity);
};


} // namespace fw

