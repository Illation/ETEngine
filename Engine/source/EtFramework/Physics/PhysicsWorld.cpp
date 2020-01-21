#include "stdafx.h"
#include "PhysicsWorld.h"

#include <btBulletDynamicsCommon.h>

#include "PhysicsManager.h"
#include "BulletETM.h"


namespace et {
namespace fw {


PhysicsWorld::~PhysicsWorld()
{
	Deinit();
}

void PhysicsWorld::Initialize()
{
	m_pWorld = PhysicsManager::GetInstance()->CreateWorld();
}

void PhysicsWorld::Deinit()
{
	if (m_pWorld == nullptr)
	{
		return;
	}

	for (int32 i = m_pWorld->getNumCollisionObjects() - 1; i >= 0; i--)
	{
		btCollisionObject* pObj = m_pWorld->getCollisionObjectArray()[i];
		btRigidBody* pBody = btRigidBody::upcast(pObj);

		if (pBody && pBody->getMotionState())
		{
			delete pBody->getMotionState();
		}

		m_pWorld->removeCollisionObject(pObj);
		delete pObj;
	}

	delete m_pWorld;
	m_pWorld = nullptr;
}

void PhysicsWorld::Update()
{
	if (!m_pWorld) return;

	m_pWorld->stepSimulation(TIME->DeltaTime(), m_MaxSubsteps);
}


} // namespace fw
} // namespace et
