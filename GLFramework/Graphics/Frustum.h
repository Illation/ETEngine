#pragma once

#include "../Helper/MathHelper.hpp"

class CameraComponent;

enum class VolumeTri
{
	OUTSIDE,
	INTERSECT,
	CONTAINS
};

class Frustum
{
public:
	Frustum();
	~Frustum();

	void Update();

	void SetToCamera(CameraComponent* pCamera);
	void SetCullTransform(glm::mat4 objectWorld);

	bool ContainsPoint(const glm::vec3 &point) const;
	bool ContainsSphere(const Sphere &sphere) const;
	VolumeTri ContainsTriangle(glm::vec3 &a, glm::vec3 &b, glm::vec3 &c);
	VolumeTri ContainsTriVolume(glm::vec3 &a, glm::vec3 &b, glm::vec3 &c, float height);

	const glm::vec3 &GetPositionOS() { return m_PositionObject; }
	const float GetFOV() { return m_FOV; }
	const float GetRadInvFOV() { return m_RadInvFOV; }

private:
	//transform to the culled objects object space and back to world space
	glm::mat4 m_CullWorld, m_CullInverse;

	//stuff in the culled objects object space
	std::vector<Plane> m_Planes;
	glm::vec3 m_PositionObject;

	float m_RadInvFOV;

	//camera parameters for locking
	glm::vec3 m_Position;
	glm::vec3 m_Forward;
	glm::vec3 m_Up;
	glm::vec3 m_Right;
	float m_NearPlane, m_FarPlane, m_FOV;
};

