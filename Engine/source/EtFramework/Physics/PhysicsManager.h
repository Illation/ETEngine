#pragma once
#include <EtCore/Helper/Singleton.h>


class btDefaultCollisionConfiguration;
class btCollisionDispatcher;
class btBroadphaseInterface;
class btSequentialImpulseConstraintSolver;

class btDiscreteDynamicsWorld;

class btBoxShape;
class btSphereShape;
class btCollisionShape;


namespace et {
namespace fw {


class PhysicsManager : public Singleton<PhysicsManager>
{
public:
	void Initialize();
	void Destroy();

	btDiscreteDynamicsWorld* CreateWorld();

	btBoxShape* CreateBoxShape(const vec3 &halfExtents);
	btSphereShape* CreateSphereShape(float radius);

private:
	bool m_IsInitialized = false;

	btDefaultCollisionConfiguration* m_pCollisionConfiguration = nullptr;
	btCollisionDispatcher* m_pDispatcher = nullptr;
	btBroadphaseInterface* m_pOverlappingPairCache = nullptr;
	btSequentialImpulseConstraintSolver* m_pSolver = nullptr;

	btDiscreteDynamicsWorld* m_pPhysicsWorld = nullptr;

	std::vector<btCollisionShape*> m_pShapes;

private:
	friend class Singleton<PhysicsManager>;

	PhysicsManager();
	virtual ~PhysicsManager();
};


} // namespace fw
} // namespace et
