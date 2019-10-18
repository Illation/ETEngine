#pragma once
#include "Frustum.h"


//-----------
// Camera
//
// Describes a perspective into a 3D world
//
class Camera final
{
public:
	// construct destruct
	//---------------------
	Camera() = default;
	~Camera() = default;

	// functionality
	//  - defering recalculations allows setting all parameters without unnecessary math being triggered, and then running Recalculate() once
	//-----------------------------------------------------------------------------------------------------------------------------------------

	void SetTransformation(vec3 const& pos, vec3 const& forward, vec3 const& up, bool const deferRecalculation = false);

	void SetIsPerspective(bool const val, bool const deferRecalculation = false);
	void SetFieldOfView(float const fov, bool const deferRecalculation = false);
	void SetSize(float const size, bool const deferRecalculation = false);

	void SetClippingPlanes(float const nearPlane, float const farPlane, bool const deferRecalculation = false);

	// accessors
	//-----------
	vec3 const& GetPosition() const { return m_Position; }
	vec3 const& GetForward() const { return m_Forward; }
	vec3 const& GetUp() const { return m_Up; }

	float GetFOV() const { return m_FieldOfView; }

	float GetNearPlane() const { return m_NearPlane; }
	float GetFarPlane() const { return m_FarPlane; }

	float GetDepthProjA() const { return m_DepthProjectionA; }
	float GetDepthProjB() const { return m_DepthProjectionB; }

	Frustum const& GetFrustum() const { return m_Frustum; }

	mat4 const& GetView() const { return m_View; }
	mat4 const& GetProj() const { return m_Projection; }
	mat4 const& GetViewInv() const { return m_ViewInverse; }
	mat4 const& GetViewProj() const { return m_ViewProjection; }
	mat4 const& GetViewProjInv() const { return m_ViewProjectionInverse; }
	mat4 const& GetStatViewProj() const { return m_StaticViewProjection; }
	mat4 const& GetStatViewProjInv() const { return m_StaticViewProjectionInverse; }

	// utility
	//----------
	void Recalculate(); // should be run if calculations where deferred

private:
	void RecalculateView();
	void RecalculateProjection();
	void RecalculateDerived();

	// Data
	///////

	// parameters
	//------------

	// world
	vec3 m_Position;
	vec3 m_Forward = vec3::FORWARD;
	vec3 m_Up = vec3::UP;

	// projection 
	bool m_IsPerspective = true;
	float m_FieldOfView = 45.f; // angle of perspective camera in degrees
	float m_Size = 25.f; // width of orthographic camera

	// depth 
	float m_NearPlane = 1.f;
	float m_FarPlane = 10.f;

	// derived
	//-----------

	// clipping
	Frustum m_Frustum;

	// reconstruct pixel position
	float m_DepthProjectionA = 0.f;
	float m_DepthProjectionB = 0.f;

	// matrices
	mat4 m_View;
	mat4 m_ViewInverse;

	mat4 m_Projection;

	mat4 m_ViewProjection;
	mat4 m_ViewProjectionInverse;

	mat4 m_StaticViewProjection;
	mat4 m_StaticViewProjectionInverse;
};

