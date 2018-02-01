#pragma once

class btDiscreteDynamicsWorld;

class PhysicsWorld
{
public:
	PhysicsWorld();
	virtual ~PhysicsWorld();

	void Initialize();
	void Update();

	btDiscreteDynamicsWorld* GetWorld() const { return m_pWorld; }

private:
	btDiscreteDynamicsWorld* m_pWorld = nullptr;

	uint32 m_MaxSubsteps = 10;
};