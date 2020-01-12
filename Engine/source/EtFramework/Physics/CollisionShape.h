#pragma once
#include <rttr/registration_friend.h>

#include <EtCore/Helper/LinkerUtils.h>


class btCollisionShape;


namespace fw {


//------------------
// CollisionShape
//
// Allows us to define different collider types for bullet in an abstract way
//
class CollisionShape
{
	RTTR_ENABLE()

public:
	virtual ~CollisionShape() = default;

	virtual CollisionShape* Clone() const = 0;
	virtual btCollisionShape* MakeBulletCollisionShape() const = 0;
};

//------------
// BoxShape
//
class BoxShape final : public CollisionShape
{
	RTTR_ENABLE(CollisionShape)
	RTTR_REGISTRATION_FRIEND
	DECLARE_FORCED_LINKING()

public:
	BoxShape() : CollisionShape() {}
	~BoxShape() = default;

	BoxShape* Clone() const override { return new BoxShape(*this); }
	btCollisionShape* MakeBulletCollisionShape() const override;

private:
	vec3 m_HalfExtents;
};

//--------------
// SphereShape
//
class SphereShape final : public CollisionShape
{
	RTTR_ENABLE(CollisionShape)
	RTTR_REGISTRATION_FRIEND

public:
	SphereShape() : CollisionShape() {}
	~SphereShape() = default;

	SphereShape* Clone() const override { return new SphereShape(*this); }
	btCollisionShape* MakeBulletCollisionShape() const override;

private:
	float m_Radius = 1.f;
};


} // namespace fw

