#pragma once
#include <EtFramework/Components/CameraComponent.h>
#include <EtFramework/Components/TransformComponent.h>

#include <EtFramework/ECS/ComponentView.h>
#include <EtFramework/ECS/EcsController.h>


namespace et {
namespace fw {


//----------------------
// CameraSyncSystemView
//
struct CameraSyncSystemView final : public ComponentView
{
	CameraSyncSystemView() : ComponentView()
	{
		Declare(camera);
		Declare(transf);
	}

	ReadAccess<CameraComponent> camera;
	ReadAccess<TransformComponent> transf;
};

//------------------
// CameraSyncSystem
//
// Extracts camera parameters into the rendering scene representation
//
class CameraSyncSystem final : public fw::System<CameraSyncSystem, CameraSyncSystemView>
{
public:
	CameraSyncSystem();

	static void OnComponentAdded(EcsController& controller, CameraComponent& component, T_EntityId const entity);
	static void OnComponentRemoved(EcsController& controller, CameraComponent& component, T_EntityId const entity);

	void Process(ComponentRange<CameraSyncSystemView>& range) override;
};


} // namespace fw
} // namespace et
