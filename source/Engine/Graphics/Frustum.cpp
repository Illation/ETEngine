#include "stdafx.hpp"
#include "Frustum.hpp"

#include "../Components/TransformComponent.hpp"
#include "../Components/CameraComponent.hpp"

void FrustumCorners::Transform(mat4 space)
{
	//move corners of the near plane
	na = vec3(space*vec4(na, 0));
	nb = vec3(space*vec4(nb, 0));
	nc = vec3(space*vec4(nc, 0));
	nd = vec3(space*vec4(nd, 0));
	//move corners of the far plane
	fa = vec3(space*vec4(fa, 0));
	fb = vec3(space*vec4(fb, 0));
	fc = vec3(space*vec4(fc, 0));
	fd = vec3(space*vec4(fd, 0));
}

Frustum::Frustum()
{
	m_Corners = FrustumCorners();
}
Frustum::~Frustum()
{
}

//create transforms to prevent transforming every triangle into world space
void Frustum::SetCullTransform(mat4 objectWorld)
{
	m_CullWorld = objectWorld;
	m_CullInverse = etm::inverse(objectWorld);
}

void Frustum::SetToCamera(CameraComponent* pCamera)
{
	m_Position = pCamera->GetTransform()->GetPosition();
	m_Forward = pCamera->GetTransform()->GetForward();
	m_Up = pCamera->GetTransform()->GetUp();
	m_Right = pCamera->GetTransform()->GetRight();
	m_NearPlane = pCamera->GetNearPlane();
	m_FarPlane = pCamera->GetFarPlane();
	m_FOV = pCamera->GetFOV();
}

void Frustum::Update()
{
	//calculate generalized relative width and aspect ratio
	float normHalfWidth = tan(etm::radians(m_FOV));
	float aspectRatio = WINDOW.GetAspectRatio();

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

	m_PositionObject = vec3(m_CullInverse*vec4(m_Position, 0));
	m_RadInvFOV = 1 / etm::radians(m_FOV);

	//construct planes
	m_Planes.clear();
	//winding in an outside perspective so the cross product creates normals pointing inward
	m_Planes.push_back(Plane(m_Corners.na, m_Corners.nb, m_Corners.nc));//Near
	m_Planes.push_back(Plane(m_Corners.fb, m_Corners.fa, m_Corners.fd));//Far 
	m_Planes.push_back(Plane(m_Corners.fa, m_Corners.na, m_Corners.fc));//Left
	m_Planes.push_back(Plane(m_Corners.nb, m_Corners.fb, m_Corners.nd));//Right
	m_Planes.push_back(Plane(m_Corners.fa, m_Corners.fb, m_Corners.na));//Top
	m_Planes.push_back(Plane(m_Corners.nc, m_Corners.nd, m_Corners.fc));//Bottom
}

VolumeCheck Frustum::ContainsPoint(const vec3 &point) const
{
	for (auto plane : m_Planes)
	{
		if (etm::dot(plane.n, point - plane.d) < 0)return VolumeCheck::OUTSIDE;
	}
	return VolumeCheck::CONTAINS;
}
VolumeCheck Frustum::ContainsSphere(const Sphere &sphere) const
{
	VolumeCheck ret = VolumeCheck::CONTAINS;
	for (auto plane : m_Planes)
	{
		float dist = etm::dot(plane.n, sphere.pos - plane.d);
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
		if (etm::dot(plane.n, a - plane.d) < 0)rejects++;
		if (etm::dot(plane.n, b - plane.d) < 0)rejects++;
		if (etm::dot(plane.n, c - plane.d) < 0)rejects++;
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
		if (etm::dot(plane.n, a - plane.d) < 0)rejects++;
		if (etm::dot(plane.n, b - plane.d) < 0)rejects++;
		if (etm::dot(plane.n, c - plane.d) < 0)rejects++;
		// if all three are outside a plane the triangle is outside the frustrum
		if (rejects >= 3)
		{
			if (etm::dot(plane.n, (a*height) - plane.d) < 0)rejects++;
			if (etm::dot(plane.n, (b*height) - plane.d) < 0)rejects++;
			if (etm::dot(plane.n, (c*height) - plane.d) < 0)rejects++;
			if (rejects >= 6)return VolumeCheck::OUTSIDE;
			else ret = VolumeCheck::INTERSECT;
		}
		// if at least one is outside the triangle intersects at least one plane
		else if (rejects > 0)ret = VolumeCheck::INTERSECT;
	}
	return ret;
}