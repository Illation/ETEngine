#pragma once

class btDiscreteDynamicsWorld;


namespace et {
namespace fw {


class PhysicsWorld
{
public:
	PhysicsWorld() = default;
	virtual ~PhysicsWorld();

	void Initialize();
	void Deinit();

	void Update();

	btDiscreteDynamicsWorld* GetWorld() const { return m_pWorld; }

private:
	btDiscreteDynamicsWorld* m_pWorld = nullptr;

	uint32 m_MaxSubsteps = 10;
};


} // namespace fw
} // namespace et
