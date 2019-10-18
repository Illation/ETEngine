#pragma once

class Camera;

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
	void Transform(mat4 space);
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

	void Update();

	void SetToCamera(Camera const& camera);
	void SetCullTransform(mat4 objectWorld);

	VolumeCheck ContainsPoint(const vec3 &point) const;
	VolumeCheck ContainsSphere(const Sphere &sphere) const;
	VolumeCheck ContainsTriangle(vec3 &a, vec3 &b, vec3 &c);
	VolumeCheck ContainsTriVolume(vec3 &a, vec3 &b, vec3 &c, float height);

	const vec3 &GetPositionOS() { return m_PositionObject; }
	const float GetFOV() { return m_FOV; }
	const float GetRadInvFOV() { return m_RadInvFOV; }

	FrustumCorners const& GetCorners() const { return m_Corners; }

private:
	//transform to the culled objects object space and back to world space
	mat4 m_CullWorld, m_CullInverse;

	//stuff in the culled objects object space
	std::vector<Plane> m_Planes;
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

