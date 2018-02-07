#include "stdafx.hpp"
#include "RigidBodyComponent.h"

#include <btBulletDynamicsCommon.h>
#include "Physics\BulletETM.h"
#include "AbstractComponent.hpp"
#include "../SceneGraph/Entity.hpp"
#include "../Physics/PhysicsWorld.h"

RigidBodyComponent::RigidBodyComponent(bool isStatic) 
	:m_IsDynamic(!isStatic)
{ }

RigidBodyComponent::~RigidBodyComponent() 
{
	GetEntity()->GetScene()->GetPhysicsWorld()->GetWorld()->removeRigidBody(m_pBody);
	delete m_pBody->getMotionState();
	delete m_pBody;
}

void RigidBodyComponent::Initialize()
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
