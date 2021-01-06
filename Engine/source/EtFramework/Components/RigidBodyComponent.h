#pragma once
#include <EtFramework/SceneGraph/ComponentDescriptor.h>
#include <EtFramework/Physics/CollisionShape.h>


class btRigidBody;


namespace et {
namespace fw {


//---------------------------------
// RigidBodyComponent
//
// Component that describes a world translation / rotation / scale, and utility functions to change them - relates to the parent entities transform
//
class RigidBodyComponent final 
{
	// definitions
	//-------------
	ECS_DECLARE_COMPONENT

	friend class RigidBodySystem;

	// construct destruct
	//--------------------
public:
	RigidBodyComponent(bool const isDynamic, float const mass, btCollisionShape* const col);

	// accessors
	//-----------
	float GetMass() const { return m_Mass; }

	vec3 GetVelocity() const;
	vec3 GetAngularVelocity() const;
	vec3 GetLocalVelocity(vec3 const& relativePoint) const;

	// functionality
	//---------------
	void ApplyImpulse(vec3 const& force);
	void ApplyImpulse(vec3 const& force, vec3 const& offset);

	void ApplyForce(vec3 const& force);
	void ApplyForce(vec3 const& force, vec3 const& offset);

	void SetVelocity(vec3 const& vel);
	void SetAngularVelocity(vec3 const& vel);

	// Data
	///////

private:
	btRigidBody* m_Body = nullptr;
	bool m_IsDynamic = false;

	btCollisionShape* m_CollisionShape = nullptr;

	float m_Mass = 0.f;
};


//---------------------------------
// RigidBodyComponentDesc
//
// Descriptor for serialization and deserialization of rigid body components
//
class RigidBodyComponentDesc final : public ComponentDescriptor<RigidBodyComponent>
{
	// definitions
	//-------------
	RTTR_ENABLE(ComponentDescriptor<RigidBodyComponent>)
	DECLARE_FORCED_LINKING()

	// construct destruct
	//--------------------
public:
	RigidBodyComponentDesc() : ComponentDescriptor<RigidBodyComponent>() {}

	RigidBodyComponentDesc& operator=(RigidBodyComponentDesc const& other);
	RigidBodyComponentDesc(RigidBodyComponentDesc const& other);
	~RigidBodyComponentDesc();

	// ComponentDescriptor interface
	//-------------------------------
	RigidBodyComponent* MakeData() override;

	// Data
	///////

	bool isDynamic = false;
	float mass = 0.f;
	CollisionShape* shape = nullptr;
};


} // namespace fw
} // namespace et
