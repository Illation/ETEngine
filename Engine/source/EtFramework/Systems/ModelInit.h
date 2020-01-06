#pragma once
#include <EtFramework/Components/ModelComponent.h>

#include <EtFramework/ECS/EcsController.h>


namespace fw {


//-------------
// ModelInit
//
// Events for adding and removing model components from the scene
//
class ModelInit final 
{
public:
	static void OnComponentAdded(EcsController& controller, ModelComponent& component, T_EntityId const entity);
	static void OnComponentRemoved(EcsController& controller, ModelComponent& component, T_EntityId const entity);
};


} // namespace fw

