#include "stdafx.h"
#include "Camera.h"


//=========
// Camera
//=========


//----------------------------
// Camera::SetTransformation
//
// Setting the transformation causes a view matrix update
//
void Camera::SetTransformation(vec3 const& pos, vec3 const& forward, vec3 const& up, bool const deferRecalculation)
{
	m_Position = pos;
	m_Forward = forward;
	m_Up = up;

	if (!deferRecalculation)
	{
		RecalculateView();
		RecalculateDerived();
	}
}

//----------------------------
// Camera::SetIsPerspective
//
void Camera::SetIsPerspective(bool const val, bool const deferRecalculation)
{
	if (val != m_IsPerspective)
	{
		m_IsPerspective = val;

		if (!deferRecalculation)
		{
			RecalculateProjection();
			RecalculateDerived();
		}
	}
}

//----------------------------
// Camera::SetFieldOfView
//
// Contributes to perspective projection
//
void Camera::SetFieldOfView(float const fov, bool const deferRecalculation)
{
	m_FieldOfView = fov;

	if (m_IsPerspective && !deferRecalculation)
	{
		RecalculateProjection();
		RecalculateDerived();
	}
}

//----------------------------
// Camera::SetSize
//
// Contributes to orthagonal projection
//
void Camera::SetSize(float const size, bool const deferRecalculation)
{
	m_Size = size;

	if (!m_IsPerspective && !deferRecalculation)
	{
		RecalculateProjection();
		RecalculateDerived();
	}
}

//----------------------------
// Camera::SetClippingPlanes
//
// Contributes to either projection
//
void Camera::SetClippingPlanes(float const nearPlane, float const farPlane, bool const deferRecalculation)
{
	m_NearPlane = nearPlane;
	m_FarPlane = farPlane;

	if (!deferRecalculation)
	{
		RecalculateProjection();
		RecalculateDerived();
	}
}

//----------------------------
// Camera::Recalculate
//
void Camera::Recalculate()
{
	RecalculateView();
	RecalculateProjection();
	RecalculateDerived();
}

//----------------------------
// Camera::RecalculateView
//
void Camera::RecalculateView()
{
	vec3 const lookAt = m_Position + m_Forward;

	m_View = etm::lookAt(m_Position, lookAt, m_Up);
	m_ViewInverse = etm::inverse(m_View);
}

//----------------------------
// Camera::RecalculateProjection
//
void Camera::RecalculateProjection()
{
	// Maybe camera should be linked to a specific viewport instead of getting the current one
	float const aspectRatio = Viewport::GetCurrentViewport()->GetAspectRatio();

	//Calculate projection
	if (m_IsPerspective)
	{
		m_Projection = etm::perspective(etm::radians(m_FieldOfView), aspectRatio, m_NearPlane, m_FarPlane);
	}
	else
	{
		ivec2 const dim = Viewport::GetCurrentViewport()->GetDimensions();

		float viewWidth = (m_Size > 0) ? m_Size * aspectRatio : dim.x;
		float viewHeight = (m_Size > 0) ? m_Size : dim.y;

		m_Projection = etm::orthographic(0.f, viewWidth, viewHeight, 0.f, m_NearPlane, m_FarPlane);
	}

	//Calculate parameters to linearize depthbuffer values
	m_DepthProjectionA = m_FarPlane / (m_FarPlane - m_NearPlane);
	m_DepthProjectionB = (-m_FarPlane * m_NearPlane) / (m_FarPlane - m_NearPlane);
}

//----------------------------
// Camera::RecalculateDerived
//
// Variables depending on both view and projection
//
void Camera::RecalculateDerived()
{
	m_ViewProjection = m_View * m_Projection;
	m_ViewProjectionInverse = etm::inverse(m_ViewProjection);

	// removes the position component from the view matrix
	m_StaticViewProjection = etm::DiscardW(m_View) * m_Projection;
	m_StaticViewProjectionInverse = etm::inverse(m_StaticViewProjection);

	// update frustum
	m_Frustum.SetCullTransform(mat4()); // Frustum will be in world space and objects need to transform themselves
	m_Frustum.SetToCamera(*this);
	m_Frustum.Update();
}
