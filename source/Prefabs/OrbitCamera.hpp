#pragma once
#include "../SceneGraph/Entity.hpp"

class FixedCamera;
class CameraComponent;

class OrbitCamera : public Entity
{
public:
	OrbitCamera();
	virtual ~OrbitCamera();

	void SetRotation(float pitch, float yaw);

protected:

	virtual void Initialize();
	virtual void Update();
	virtual void Start();

private:
	float m_Distance = 10;
	float m_TotalPitch = 0, 
		m_TotalYaw = 0;
	float m_MoveSpeed = 2.5, 
		m_RotationSpeed = 0.2f, 
		m_SpeedMultiplier = 3;
	CameraComponent *m_pCamera = nullptr;
	Entity* m_pCamParent = nullptr;
	FixedCamera* m_pFixCam = nullptr;

	vec3 m_RotationCenter = vec3(0, 0, 0);

private:
	// -------------------------
	// Disabling default copy constructor and default 
	// assignment operator.
	// -------------------------
	OrbitCamera(const OrbitCamera& yRef);
	OrbitCamera& operator=(const OrbitCamera& yRef);
};

