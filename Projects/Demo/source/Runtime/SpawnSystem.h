#pragma once
#include <EtFramework/ECS/ComponentView.h>
#include <EtFramework/ECS/EcsController.h>
#include <EtFramework/Components/TransformComponent.h>

#include <Common/SpawnComponent.h>


namespace demo {


//---------------------------
// SpawnSystemView
//
// ECS access pattern for spawn behavior
//
struct SpawnSystemView final : public fw::ComponentView
{
	SpawnSystemView() : fw::ComponentView()
	{
		Declare(spawner);
		Declare(transform);
	}

	WriteAccess<SpawnComponent> spawner;
	ReadAccess<fw::TransformComponent> transform;
};

//---------------------------
// SpawnSystem
//
// Allow the user to move around in the 3D world with keyboard and mouse
//
class SpawnSystem final : public fw::System<SpawnSystem, SpawnSystemView>
{
public:
	SpawnSystem();

	void Process(fw::ComponentRange<SpawnSystemView>& range) override;
};


} // namespace demo

