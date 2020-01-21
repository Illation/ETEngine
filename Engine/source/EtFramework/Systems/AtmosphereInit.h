#pragma once
#include <EtFramework/Components/AtmosphereComponent.h>

#include <EtFramework/ECS/EcsController.h>


namespace et {
namespace fw {


//----------------
// AtmosphereInit
//
// Events for adding and removing atmosphere components from the scene
//
class AtmosphereInit final
{
public:
	static void OnComponentAdded(EcsController& controller, AtmosphereComponent& component, T_EntityId const entity);
	static void OnComponentRemoved(EcsController& controller, AtmosphereComponent& component, T_EntityId const entity);
};


} // namespace fw
} // namespace et
