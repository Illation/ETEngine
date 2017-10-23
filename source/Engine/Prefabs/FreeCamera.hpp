#pragma once
#include "../SceneGraph/Entity.hpp"

class CameraComponent;

class FreeCamera : public Entity
{
public:
	FreeCamera();
	virtual ~FreeCamera();

	void SetRotation(float pitch, float yaw);

protected:

	virtual void Initialize();
	virtual void Update();

private:

	float m_TotalPitch = 0, 
		m_TotalYaw = 0;
	float m_MoveSpeed = 2.5, 
		m_RotationSpeed = 0.2f, 
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

