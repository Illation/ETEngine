#pragma once


// forward
class CameraComponent;


//--------------------------
// FreeCamera
//
// controller that allows unrestricted camera movement through a scene using WASD
//
class FreeCamera
{
	// definitions
	//-------------
	static float const s_MoveSpeed;
	static float const s_RotationSpeed;
	static float const s_Acceleration;

	// construct destruct
	//--------------------
public:
	FreeCamera();
	virtual ~FreeCamera() = default;

	void SetCameraComponent(CameraComponent* const camComp) { m_Camera = camComp; }

	// functionality
	//---------------
	void Reset();
	void Update();

	// Data
	///////
	
private:
	CameraComponent* m_Camera = nullptr;

	float m_TotalPitch;
	float m_TotalYaw;
	float m_SpeedMultiplier;
	vec3 m_Move;
};

