#pragma once

namespace et {
namespace render {


class Camera;
class Viewport;

enum class VolumeCheck
{
	OUTSIDE,
	INTERSECT,
	CONTAINS
};

struct FrustumCorners
{
	//Utility to transform frustum into any objects space
	//Useful for complex frustum culling operations
	void Transform(mat4 const& space);
	//near plane
	vec3 na;
	vec3 nb; 
	vec3 nc;
	vec3 nd;
	//far plane
	vec3 fa;
	vec3 fb;
	vec3 fc;
	vec3 fd;
};

class Frustum
{
public:
	Frustum();
	~Frustum();

	void Update(rhi::Viewport const* const viewport);

	void SetToCamera(Camera const& camera);
	void SetCullTransform(mat4 const& objectWorld);

	VolumeCheck ContainsPoint(const vec3 &point) const;
	VolumeCheck ContainsSphere(math::Sphere const& sphere) const;
	VolumeCheck ContainsTriangle(vec3 &a, vec3 &b, vec3 &c);
	VolumeCheck ContainsTriVolume(vec3 &a, vec3 &b, vec3 &c, float height);

	vec3 const& GetPositionOS() const { return m_PositionObject; }
	float GetFOV() const { return m_FOV; }
	float GetRadInvFOV() const { return m_RadInvFOV; }

	FrustumCorners const& GetCorners() const { return m_Corners; }

private:
	//transform to the culled objects object space and back to world space
	mat4 m_CullWorld, m_CullInverse;

	//stuff in the culled objects object space
	std::vector<math::Plane> m_Planes;
	FrustumCorners m_Corners;
	vec3 m_PositionObject;

	float m_RadInvFOV;

	//camera parameters for locking
	vec3 m_Position;
	vec3 m_Forward;
	vec3 m_Up;
	vec3 m_Right;
	float m_NearPlane, m_FarPlane, m_FOV;
};


} // namespace render
} // namespace et
