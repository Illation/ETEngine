#pragma once
#include <EtFramework/SceneGraph/Entity.h>


class CameraComponent;

class FreeCamera : public Entity
{
public:
	FreeCamera();
	virtual ~FreeCamera() = default;

	void SetRotation(float pitch, float yaw);

protected:

	virtual void Init();
	virtual void Update();

private:

	float m_TotalPitch = 0, 
		m_TotalYaw = 0;
	float m_MoveSpeed = 2.5, 
		m_RotationSpeed = 0.0032f, 
		m_SpeedMultiplier = 1;
	CameraComponent *m_pCamera = nullptr;

	vec3 m_Move;
	float m_Acceleration = 5;

private:
	// -------------------------
	// Disabling default copy constructor and default 
	// assignment operator.
	// -------------------------
	FreeCamera(const FreeCamera& yRef);
	FreeCamera& operator=(const FreeCamera& yRef);
};

