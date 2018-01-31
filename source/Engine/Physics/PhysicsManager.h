#pragma once
#include "Singleton.hpp"

class btDefaultCollisionConfiguration;
class btCollisionDispatcher;
class btBroadphaseInterface;
class btSequentialImpulseConstraintSolver;

class btDiscreteDynamicsWorld;

class btBoxShape;

class PhysicsManager : public Singleton<PhysicsManager>
{
public:
	void Initialize();
	void Destroy();

	btDiscreteDynamicsWorld* CreateWorld();

	btBoxShape* CreateBoxShape(const vec3 &halfExtents);

private:
	bool m_IsInitialized = false;

	btDefaultCollisionConfiguration* m_pCollisionConfiguration = nullptr;
	btCollisionDispatcher* m_pDispatcher = nullptr;
	btBroadphaseInterface* m_pOverlappingPairCache = nullptr;
	btSequentialImpulseConstraintSolver* m_pSolver = nullptr;

	btDiscreteDynamicsWorld* m_pPhysicsWorld = nullptr;

private:
	friend class Singleton<PhysicsManager>;

	PhysicsManager();
	virtual ~PhysicsManager();
};