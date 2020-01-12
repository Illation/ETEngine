#include "stdafx.h"
#include "CollisionShape.h"

#include <btBulletDynamicsCommon.h>

#include <EtFramework/Physics/BulletETM.h>
#include <EtFramework/Physics/PhysicsWorld.h>
#include <EtFramework/Physics/PhysicsManager.h>


//=================
// Collision Shape
//=================


// reflection
//------------

RTTR_REGISTRATION
{
	using namespace rttr;

	// box collider
	registration::class_<fw::BoxShape>("box collider shape")
		.constructor<fw::BoxShape const&>()
		.constructor<>()(rttr::detail::as_object())
		.property("half extents", &fw::BoxShape::m_HalfExtents);

	rttr::type::register_converter_func([](fw::BoxShape& shape, bool& ok) -> fw::CollisionShape*
	{
		ok = true;
		return new fw::BoxShape(shape);
	});

	// sphere collider
	registration::class_<fw::SphereShape>("sphere collider shape")
		.constructor<fw::SphereShape const&>()
		.constructor<>()(rttr::detail::as_object())
		.property("radius", &fw::SphereShape::m_Radius);

	rttr::type::register_converter_func([](fw::SphereShape& shape, bool& ok) -> fw::CollisionShape*
	{
		ok = true;
		return new fw::SphereShape(shape);
	});
}

DEFINE_FORCED_LINKING(fw::BoxShape) // force the linker to include this unit


namespace fw {


//------------------------------------
// BoxShape::MakeBulletCollisionShape
//
btCollisionShape* BoxShape::MakeBulletCollisionShape() const
{
	return PhysicsManager::GetInstance()->CreateBoxShape(m_HalfExtents);
}

//---------------------------------------
// SphereShape::MakeBulletCollisionShape
//
btCollisionShape* SphereShape::MakeBulletCollisionShape() const
{
	return PhysicsManager::GetInstance()->CreateSphereShape(m_Radius);
}


} // namespace fw
