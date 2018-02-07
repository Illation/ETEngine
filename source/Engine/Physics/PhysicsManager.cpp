#include "stdafx.hpp"
#include "PhysicsManager.h"

#include <btBulletDynamicsCommon.h>
#include "BulletETM.h"

PhysicsManager::PhysicsManager() { }
PhysicsManager::~PhysicsManager() { Destroy(); }

void PhysicsManager::Initialize()
{
	if (m_IsInitialized)return;

	m_pCollisionConfiguration = new btDefaultCollisionConfiguration();
	m_pDispatcher = new btCollisionDispatcher(m_pCollisionConfiguration);
	m_pOverlappingPairCache = new btDbvtBroadphase();
	m_pSolver = new btSequentialImpulseConstraintSolver;

	m_IsInitialized = true;
}

void PhysicsManager::Destroy()
{
	if (!m_IsInitialized)return;
	m_IsInitialized = false;

	for (auto shape : m_pShapes)delete shape;
	m_pShapes.clear();

	delete m_pSolver;
	delete m_pOverlappingPairCache;
	delete m_pDispatcher;
	delete m_pCollisionConfiguration;
}

btDiscreteDynamicsWorld* PhysicsManager::CreateWorld()
{
	btDiscreteDynamicsWorld* pWorld = new btDiscreteDynamicsWorld(m_pDispatcher, m_pOverlappingPairCache, m_pSolver, m_pCollisionConfiguration);
	return pWorld;
}

btBoxShape* PhysicsManager::CreateBoxShape(const vec3 &halfExtents)
{
	btBoxShape* box = new btBoxShape(ToBtVec3(halfExtents));
	m_pShapes.push_back(box);
	return box;
}

btSphereShape* PhysicsManager::CreateSphereShape(float radius)
{
	btSphereShape* sphere = new btSphereShape(btScalar(radius));
	m_pShapes.push_back(sphere);
	return sphere;
}
