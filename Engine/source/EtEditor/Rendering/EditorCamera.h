#pragma once


// forward
class CameraComponent;
class Viewport;


//---------------------------------
// EditorCamera
//
// Componentless implementation of a free moving camera
//
class EditorCamera final
{
	// definitions
	//--------------
	static float const s_MoveSpeed;
	static float const s_Accelleration;
	static float const s_RotationSpeed;

	// construct destruct
	//--------------------
public:
	EditorCamera();
	~EditorCamera() = default;

	// functionality
	//----------------
	void Update(Camera const& currentCamera);
	void PopulateCamera(Camera& target, Viewport const* const viewport) const;

	void ImitateComponent(CameraComponent const* const comp);

	void SetEnabled(bool const val) { m_IsEnabled = val; }

	// Data
	///////
private:

	bool m_IsEnabled = false;

	// movement
	vec3 m_Movement;
	float m_SpeedMultiplier = 1.f;

	float m_TotalPitch = 0.f;
	float m_TotalYaw = 0.f;

	// camera parameters
	float m_FieldOfView = 45.f; // angle of perspective camera in degrees
	float m_NearPlane = 0.1f;
	float m_FarPlane = 1000.f;

	// transform parameters
	vec3 m_Position;
	vec3 m_Forward;
	vec3 m_Up;

	// utility
	vec3 m_Right;
};