#pragma once
#include <EtCore/Reflection/Registration.h>

#include <EtCore/Util/LinkerUtils.h>


class btCollisionShape;
namespace et {
	REGISTRATION_NS(fw)
}


namespace et {
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
	REGISTRATION_FRIEND_NS(et::fw)
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
	REGISTRATION_FRIEND_NS(fw)

public:
	SphereShape() : CollisionShape() {}
	~SphereShape() = default;

	SphereShape* Clone() const override { return new SphereShape(*this); }
	btCollisionShape* MakeBulletCollisionShape() const override;

private:
	float m_Radius = 1.f;
};


} // namespace fw
} // namespace et

