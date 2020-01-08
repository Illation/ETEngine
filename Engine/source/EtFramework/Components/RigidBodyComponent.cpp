#include "stdafx.h"
#include "RigidBodyComponent.h"

#include <btBulletDynamicsCommon.h>

#include "AbstractComponent.h"

#include <EtFramework/Physics/BulletETM.h>
#include <EtFramework/Physics/PhysicsWorld.h>
#include <EtFramework/Physics/PhysicsManager.h>
#include <EtFramework/SceneGraph/Entity.h>


// deprecated
//------------


RigidBodyComponent::RigidBodyComponent(bool isStatic) 
	:m_IsDynamic(!isStatic)
{ }

RigidBodyComponent::~RigidBodyComponent() 
{
	delete m_pBody->getMotionState();
	delete m_pBody;
}

void RigidBodyComponent::Init()
{
	btAssert((!m_pCollisionShape || m_pCollisionShape->getShapeType() != INVALID_SHAPE_PROXYTYPE));

	//rigidbody is dynamic if and only if mass is non zero, otherwise static
	assert(!m_IsDynamic || m_Mass > 0);

	btTransform startTransform;
	startTransform.setOrigin(ToBtVec3(TRANSFORM->GetWorldPosition()));
	startTransform.setRotation(ToBtQuat(TRANSFORM->GetWorldRotation()));

	btVector3 localInertia(0, 0, 0);
	if (m_IsDynamic)
		m_pCollisionShape->calculateLocalInertia(m_Mass, localInertia);

	btDefaultMotionState* myMotionState = new btDefaultMotionState(startTransform);

	btRigidBody::btRigidBodyConstructionInfo cInfo(m_Mass, myMotionState, m_pCollisionShape, localInertia);

	m_pBody = new btRigidBody(cInfo);

	m_pBody->setUserIndex(-1);

	GetEntity()->GetScene()->GetPhysicsWorld()->GetWorld()->addRigidBody(m_pBody);
}

void RigidBodyComponent::Deinit()
{
	GetEntity()->GetScene()->GetPhysicsWorld()->GetWorld()->removeRigidBody(m_pBody);
}

void RigidBodyComponent::SetPosition(const vec3 &pos)
{
	btTransform trans;
	if (m_pBody && m_pBody->getMotionState())
	{
		m_pBody->getMotionState()->getWorldTransform(trans);
		trans.setOrigin(ToBtVec3(pos));
		m_pBody->getMotionState()->setWorldTransform(trans);
	}
}

vec3 RigidBodyComponent::GetPosition()
{
	btTransform trans;
	if (m_pBody && m_pBody->getMotionState())
	{
		m_pBody->getMotionState()->getWorldTransform(trans);
		return ToEtmVec3(trans.getOrigin());
	}
	return vec3(0);
}

void RigidBodyComponent::SetRotation(const quat &rot)
{
	btTransform trans;
	if (m_pBody && m_pBody->getMotionState())
	{
		m_pBody->getMotionState()->getWorldTransform(trans);
		trans.setRotation(ToBtQuat(rot));
		m_pBody->getMotionState()->setWorldTransform(trans);
	}
}

quat RigidBodyComponent::GetRotation()
{
	btTransform trans;
	if (m_pBody && m_pBody->getMotionState())
	{
		m_pBody->getMotionState()->getWorldTransform(trans);
		return ToEtmQuat(trans.getRotation());
	}
	return quat();
}

void RigidBodyComponent::ApplyImpulse(const vec3 &force, const vec3 &offset)
{
	if (m_pBody)
	{
		if (etm::nearEqualsV(offset, vec3::ZERO))
		{
			m_pBody->applyCentralImpulse(ToBtVec3(force));
		}
		else
		{
			m_pBody->applyImpulse(ToBtVec3(force), ToBtVec3(offset));
		}
	}
}

void RigidBodyComponent::ApplyForce(const vec3 &force, const vec3 &offset /*= vec3(0)*/)
{
	if (m_pBody)
	{
		if (etm::nearEqualsV(offset, vec3::ZERO))
		{
			m_pBody->applyCentralForce(ToBtVec3(force));
		}
		else
		{
			m_pBody->applyForce(ToBtVec3(force), ToBtVec3(offset));
		}
	}
}


// reflection
//------------

RTTR_REGISTRATION
{
	using namespace rttr;

	// component
	registration::class_<fw::RigidBodyComponent>("rigid body component");

	// box collider
	registration::class_<fw::RigidBodyComponentDesc::BoxShape>("box collider shape")
		.constructor<fw::RigidBodyComponentDesc::BoxShape const&>()
		.constructor<>()(rttr::detail::as_object())
		.property("half extents", &fw::RigidBodyComponentDesc::BoxShape::m_HalfExtents);

	rttr::type::register_converter_func([](fw::RigidBodyComponentDesc::BoxShape& shape, bool& ok) -> fw::RigidBodyComponentDesc::CollisionShape*
	{
		ok = true;
		return new fw::RigidBodyComponentDesc::BoxShape(shape);
	});

	// sphere collider
	registration::class_<fw::RigidBodyComponentDesc::SphereShape>("sphere collider shape")
		.constructor<fw::RigidBodyComponentDesc::SphereShape const&>()
		.constructor<>()(rttr::detail::as_object())
		.property("radius", &fw::RigidBodyComponentDesc::SphereShape::m_Radius);

	rttr::type::register_converter_func([](fw::RigidBodyComponentDesc::SphereShape& shape, bool& ok) -> fw::RigidBodyComponentDesc::CollisionShape*
	{
		ok = true;
		return new fw::RigidBodyComponentDesc::SphereShape(shape);
	});

	// component descriptor
	registration::class_<fw::RigidBodyComponentDesc>("rigid body comp desc")
		.constructor<fw::RigidBodyComponentDesc const&>()
		.constructor<>()(rttr::detail::as_object())
		.property("is dynamic", &fw::RigidBodyComponentDesc::isDynamic)
		.property("mass", &fw::RigidBodyComponentDesc::mass)
		.property("shape", &fw::RigidBodyComponentDesc::shape);

	rttr::type::register_converter_func([](fw::RigidBodyComponentDesc& descriptor, bool& ok) -> fw::I_ComponentDescriptor*
	{
		ok = true;
		return new fw::RigidBodyComponentDesc(descriptor);
	});
}

// component registration
//------------------------

ECS_REGISTER_COMPONENT(fw::RigidBodyComponent);


namespace fw {


//======================
// Rigid Body Component 
//======================


//-----------------------
// RigidBodyComponent::c-tor
//
RigidBodyComponent::RigidBodyComponent(bool const isDynamic, float const mass, btCollisionShape* const col)
	: m_IsDynamic(isDynamic)
	, m_Mass(mass)
	, m_CollisionShape(col)
{ }

//----------------------------------
// RigidBodyComponent::GetVelocity
//
vec3 RigidBodyComponent::GetVelocity() const
{
	ET_ASSERT(m_Body != nullptr);
	return ToEtmVec3(m_Body->getLinearVelocity());
}

//----------------------------------------
// RigidBodyComponent::GetAngularVelocity
//
vec3 RigidBodyComponent::GetAngularVelocity() const
{
	ET_ASSERT(m_Body != nullptr);
	return ToEtmVec3(m_Body->getAngularVelocity());
}

//--------------------------------------
// RigidBodyComponent::GetLocalVelocity
//
vec3 RigidBodyComponent::GetLocalVelocity(vec3 const& relativePoint) const
{
	ET_ASSERT(m_Body != nullptr);
	return ToEtmVec3(m_Body->getVelocityInLocalPoint(ToBtVec3(relativePoint)));
}

//----------------------------------
// RigidBodyComponent::ApplyImpulse
//
void RigidBodyComponent::ApplyImpulse(vec3 const& force)
{
	ET_ASSERT(m_Body != nullptr);
	m_Body->applyCentralImpulse(ToBtVec3(force));
}

//----------------------------------
// RigidBodyComponent::ApplyImpulse
//
void RigidBodyComponent::ApplyImpulse(vec3 const& force, vec3 const& offset)
{
	ET_ASSERT(m_Body != nullptr);
	m_Body->applyImpulse(ToBtVec3(force), ToBtVec3(offset));
}

//--------------------------------
// RigidBodyComponent::ApplyForce
//
void RigidBodyComponent::ApplyForce(vec3 const& force)
{
	ET_ASSERT(m_Body != nullptr);
	m_Body->applyCentralForce(ToBtVec3(force));
}

//--------------------------------
// RigidBodyComponent::ApplyForce
//
void RigidBodyComponent::ApplyForce(vec3 const& force, vec3 const& offset)
{
	ET_ASSERT(m_Body != nullptr);
	m_Body->applyForce(ToBtVec3(force), ToBtVec3(offset));
}

//---------------------------------
// RigidBodyComponent::SetVelocity
//
void RigidBodyComponent::SetVelocity(vec3 const& vel)
{
	ET_ASSERT(m_Body != nullptr);
	m_Body->setLinearVelocity(ToBtVec3(vel));
}

//----------------------------------------
// RigidBodyComponent::SetAngularVelocity
//
void RigidBodyComponent::SetAngularVelocity(vec3 const& vel)
{
	ET_ASSERT(m_Body != nullptr);
	m_Body->setAngularVelocity(ToBtVec3(vel));
}


//=================================
// Rigid Body Component Descriptor
//=================================


//------------------------------------------------------------
// RigidBodyComponentDesc::BoxShape::MakeBulletCollisionShape
//
btCollisionShape* RigidBodyComponentDesc::BoxShape::MakeBulletCollisionShape() const
{
	return PhysicsManager::GetInstance()->CreateBoxShape(m_HalfExtents);
}

//---------------------------------------------------------------
// RigidBodyComponentDesc::SphereShape::MakeBulletCollisionShape
//
btCollisionShape* RigidBodyComponentDesc::SphereShape::MakeBulletCollisionShape() const
{
	return PhysicsManager::GetInstance()->CreateSphereShape(m_Radius);
}

//----------------------------------
// RigidBodyComponentDesc::MakeData
//
// Create a rigid body component from a descriptor
//
RigidBodyComponent* RigidBodyComponentDesc::MakeData()
{
	ET_ASSERT(shape != nullptr);
	return new RigidBodyComponent(isDynamic, mass, shape->MakeBulletCollisionShape());
}


} // namespace fw
