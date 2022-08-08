#include "stdafx.h"
#include "Camera.h"

#include <EtCore/Util/DebugUtilFwd.h>

#if ET_CT_IS_ENABLED(ET_CT_DBG_UTIL)
#	include <EtRendering/GlobalRenderingSystems/GlobalRenderingSystems.h>
#endif


namespace et {
namespace render {


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
// Camera::SetViewport
//
void Camera::SetViewport(Ptr<rhi::Viewport> const viewport, bool const deferRecalculation)
{
	m_Viewport = viewport;

	if (!deferRecalculation)
	{
		RecalculateProjection();
		RecalculateDerived();
	}
}

//----------------------------
// Camera::SetViewport
//
// input vector should range from 0 to 1 with 0 being in the top left corner
// depth of 0 is at the near plane, 1 at the far plane
//
vec3 Camera::ProjectIntoWorldSpace(vec2 const screenSpaceNorm, float const depth) const
{
	vec2 const ndcPos(screenSpaceNorm.x * 2.f - 1.f, -(screenSpaceNorm.y * 2.f - 1.f));

	vec4 const viewPosH = m_ProjectionInverse * vec4(ndcPos, (2.f * depth) - 1.f, 1.f);
	vec3 const viewPos = viewPosH.xyz / viewPosH.w;

	return (m_ViewInverse * vec4(viewPos, 1.f)).xyz;
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

	m_View = math::lookAt(m_Position, lookAt, m_Up);
	m_ViewInverse = math::inverse(m_View);
}

//----------------------------
// Camera::RecalculateProjection
//
void Camera::RecalculateProjection()
{
	if (m_Viewport == nullptr)
	{
		return;
	}

	// Maybe camera should be linked to a specific viewport instead of getting the current one
	float const aspectRatio = m_Viewport->GetAspectRatio();

	//Calculate projection
	if (m_IsPerspective)
	{
		m_Projection = math::perspective(math::radians(m_FieldOfView), aspectRatio, m_NearPlane, m_FarPlane);
	}
	else
	{
		ivec2 const dim = m_Viewport->GetDimensions();

		float viewWidth = (m_Size > 0) ? m_Size * aspectRatio : dim.x;
		float viewHeight = (m_Size > 0) ? m_Size : dim.y;

		m_Projection = math::orthographic(0.f, viewWidth, viewHeight, 0.f, m_NearPlane, m_FarPlane);
	}

	m_ProjectionInverse = math::inverse(m_Projection);

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
	if (m_Viewport == nullptr)
	{
		return;
	}

	m_ViewProjection = m_View * m_Projection;
	m_ViewProjectionInverse = math::inverse(m_ViewProjection);

	// removes the position component from the view matrix
	m_StaticViewProjection = math::DiscardW(m_View) * m_Projection;
	m_StaticViewProjectionInverse = math::inverse(m_StaticViewProjection);

	// update frustum
	m_Frustum.SetCullTransform(mat4()); // Frustum will be in world space and objects need to transform themselves

#if ET_CT_IS_ENABLED(ET_CT_DBG_UTIL)
	if (!(RenderingSystems::Instance()->GetDebugVars().IsFrustumFrozen()))
	{ 
#endif
		m_Frustum.SetToCamera(*this);
#if ET_CT_IS_ENABLED(ET_CT_DBG_UTIL)
	}
#endif

	m_Frustum.Update(m_Viewport.Get());
}


} // namespace render
} // namespace et
