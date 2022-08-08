#include "stdafx.h"
#include "Frustum.h"

#include "Camera.h"


namespace et {
namespace render {


void FrustumCorners::Transform(mat4 const& space)
{
	//move corners of the near plane
	na = (space*vec4(na, 0)).xyz;
	nb = (space*vec4(nb, 0)).xyz;
	nc = (space*vec4(nc, 0)).xyz;
	nd = (space*vec4(nd, 0)).xyz;
	//move corners of the far plane
	fa = (space*vec4(fa, 0)).xyz;
	fb = (space*vec4(fb, 0)).xyz;
	fc = (space*vec4(fc, 0)).xyz;
	fd = (space*vec4(fd, 0)).xyz;
}

Frustum::Frustum()
{
	m_Corners = FrustumCorners();
}
Frustum::~Frustum()
{
}

//create transforms to prevent transforming every triangle into world space
void Frustum::SetCullTransform(mat4 const& objectWorld)
{
	m_CullWorld = objectWorld;
	m_CullInverse = math::inverse(objectWorld);
}

void Frustum::SetToCamera(Camera const& camera)
{
	m_Position = camera.GetPosition();

	m_Forward = camera.GetForward();
	m_Up = camera.GetUp();
	m_Right = math::cross(m_Up, m_Forward);

	m_NearPlane = camera.GetNearPlane();
	m_FarPlane = camera.GetFarPlane();
	m_FOV = camera.GetFOV();
}

void Frustum::Update(rhi::Viewport const* const viewport)
{
	//calculate generalized relative width and aspect ratio
	float normHalfWidth = tan(math::radians(m_FOV));
	float aspectRatio = viewport->GetAspectRatio();

	//calculate width and height for near and far plane
	float nearHW = normHalfWidth*m_NearPlane;
	float nearHH = nearHW / aspectRatio;
	float farHW = normHalfWidth*m_FarPlane;// *0.5f;
	float farHH = farHW / aspectRatio;

	//calculate near and far plane centers
	auto nCenter = m_Position + m_Forward*m_NearPlane;
	auto fCenter = m_Position + m_Forward*m_FarPlane *0.5f;

	//construct corners of the near plane in the culled objects world space
	m_Corners.na = nCenter + m_Up*nearHH - m_Right*nearHW;
	m_Corners.nb = nCenter + m_Up*nearHH + m_Right*nearHW;
	m_Corners.nc = nCenter - m_Up*nearHH - m_Right*nearHW;
	m_Corners.nd = nCenter - m_Up*nearHH + m_Right*nearHW;
	//construct corners of the far plane
	m_Corners.fa = fCenter + m_Up*farHH - m_Right*farHW;
	m_Corners.fb = fCenter + m_Up*farHH + m_Right*farHW;
	m_Corners.fc = fCenter - m_Up*farHH - m_Right*farHW;
	m_Corners.fd = fCenter - m_Up*farHH + m_Right*farHW;

	m_Corners.Transform(m_CullInverse);

	m_PositionObject = (m_CullInverse*vec4(m_Position, 0)).xyz;
	m_RadInvFOV = 1 / math::radians(m_FOV);

	//construct planes
	m_Planes.clear();
	//winding in an outside perspective so the cross product creates normals pointing inward
	m_Planes.push_back(math::Plane(m_Corners.na, m_Corners.nb, m_Corners.nc));//Near
	m_Planes.push_back(math::Plane(m_Corners.fb, m_Corners.fa, m_Corners.fd));//Far 
	m_Planes.push_back(math::Plane(m_Corners.fa, m_Corners.na, m_Corners.fc));//Left
	m_Planes.push_back(math::Plane(m_Corners.nb, m_Corners.fb, m_Corners.nd));//Right
	m_Planes.push_back(math::Plane(m_Corners.fa, m_Corners.fb, m_Corners.na));//Top
	m_Planes.push_back(math::Plane(m_Corners.nc, m_Corners.nd, m_Corners.fc));//Bottom
}

VolumeCheck Frustum::ContainsPoint(const vec3 &point) const
{
	for (auto plane : m_Planes)
	{
		if (math::dot(plane.n, point - plane.d) < 0)return VolumeCheck::OUTSIDE;
	}
	return VolumeCheck::CONTAINS;
}
VolumeCheck Frustum::ContainsSphere(math::Sphere const& sphere) const
{
	VolumeCheck ret = VolumeCheck::CONTAINS;
	for (auto plane : m_Planes)
	{
		float dist = math::dot(plane.n, sphere.pos - plane.d);
		if (dist < -sphere.radius)return VolumeCheck::OUTSIDE;
		else if (dist < 0) ret = VolumeCheck::INTERSECT;
	}
	return ret;
}
//this method will treat triangles as intersecting even though they may be outside
//but it is faster then performing a proper intersection test with every plane
//and it does not reject triangles that are inside but with all corners outside
VolumeCheck Frustum::ContainsTriangle(vec3 &a, vec3 &b, vec3 &c)
{
	VolumeCheck ret = VolumeCheck::CONTAINS;
	for (auto plane : m_Planes)
	{
		char rejects = 0;
		if (math::dot(plane.n, a - plane.d) < 0)rejects++;
		if (math::dot(plane.n, b - plane.d) < 0)rejects++;
		if (math::dot(plane.n, c - plane.d) < 0)rejects++;
		// if all three are outside a plane the triangle is outside the frustrum
		if (rejects >= 3)return VolumeCheck::OUTSIDE;
		// if at least one is outside the triangle intersects at least one plane
		else if (rejects > 0)ret = VolumeCheck::INTERSECT;
	}
	return ret;
}
//same as above but with a volume generated above the triangle
VolumeCheck Frustum::ContainsTriVolume(vec3 &a, vec3 &b, vec3 &c, float height)
{
	VolumeCheck ret = VolumeCheck::CONTAINS;
	for (auto plane : m_Planes)
	{
		char rejects = 0;
		if (math::dot(plane.n, a - plane.d) < 0)rejects++;
		if (math::dot(plane.n, b - plane.d) < 0)rejects++;
		if (math::dot(plane.n, c - plane.d) < 0)rejects++;
		// if all three are outside a plane the triangle is outside the frustrum
		if (rejects >= 3)
		{
			if (math::dot(plane.n, (a*height) - plane.d) < 0)rejects++;
			if (math::dot(plane.n, (b*height) - plane.d) < 0)rejects++;
			if (math::dot(plane.n, (c*height) - plane.d) < 0)rejects++;
			if (rejects >= 6)return VolumeCheck::OUTSIDE;
			else ret = VolumeCheck::INTERSECT;
		}
		// if at least one is outside the triangle intersects at least one plane
		else if (rejects > 0)ret = VolumeCheck::INTERSECT;
	}
	return ret;
}


} // namespace render
} // namespace et
