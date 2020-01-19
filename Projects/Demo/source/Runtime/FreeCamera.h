#pragma once
#include <EtFramework/ECS/ComponentView.h>
#include <EtFramework/ECS/EcsController.h>

#include <EtFramework/Components/CameraComponent.h>
#include <EtFramework/Components/TransformComponent.h>


namespace et {
namespace demo {


//---------------------------
// FreeCameraComponent
//
// describes the state of editor camera behavior
//
struct FreeCameraComponent final
{
	ECS_DECLARE_COMPONENT

public:
	// inherent
	vec3 movement;

	float speedMultiplier = 1.f;

	float totalPitch = 0.f;
	float totalYaw = 0.f;
};


//---------------------------
// FreeCameraSystemView
//
// ECS access pattern for editor camera behavior
//
struct FreeCameraSystemView final : public fw::ComponentView
{
	FreeCameraSystemView() : fw::ComponentView()
	{
		Declare(camera);
		Declare(transform);
		Include<fw::CameraComponent>();
	}

	WriteAccess<FreeCameraComponent> camera;
	WriteAccess<fw::TransformComponent> transform;
};

//---------------------------
// FreeCameraSystem
//
// Allow the user to move around in the 3D world with keyboard and mouse
//
class FreeCameraSystem final : public fw::System<FreeCameraSystem, FreeCameraSystemView>
{
	static float const s_MoveSpeed;
	static float const s_Accelleration;
	static float const s_RotationSpeed;

public:
	FreeCameraSystem();

	void Process(fw::ComponentRange<FreeCameraSystemView>& ramge) override;
};


} // namespace demo
} // namespace et
