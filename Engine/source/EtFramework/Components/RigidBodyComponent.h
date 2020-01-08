#pragma once
#include "AbstractComponent.h"

#include <rttr/registration_friend.h>


class btRigidBody;
class btCollisionShape;


class RigidBodyComponent final : public AbstractComponent
{
public:
	RigidBodyComponent(bool isStatic = false);
	~RigidBodyComponent();

	void Init() override;
	void Deinit() override;
	void Update() override {}

	float GetMass() const { return m_Mass; }
	void SetMass(float val) { m_Mass = val; }

	void SetCollisionShape(btCollisionShape* val) { m_pCollisionShape = val; }

	void SetPosition(const vec3 &pos);
	vec3 GetPosition();
	void SetRotation(const quat &rot);
	quat GetRotation();

	void ApplyImpulse(const vec3 &force, const vec3 &offset = vec3(0));
	void ApplyForce(const vec3 &force, const vec3 &offset = vec3(0));

private:
	btRigidBody* m_pBody = nullptr;

	btCollisionShape* m_pCollisionShape = nullptr;
	float m_Mass = 0;

	bool m_IsDynamic = false;
};



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

public:

	//------------------
	// CollisionShape
	//
	// Allows us to define different collider types for bullet in an abstract way
	//
	class CollisionShape
	{
		RTTR_ENABLE()

	public:
		virtual ~CollisionShape() = default;
		virtual btCollisionShape* MakeBulletCollisionShape() const = 0;
	};

	//------------
	// BoxShape
	//
	class BoxShape final : public CollisionShape
	{
		RTTR_ENABLE(CollisionShape)
		RTTR_REGISTRATION_FRIEND

	public:
		BoxShape() : CollisionShape() {}
		~BoxShape() = default;

		btCollisionShape* MakeBulletCollisionShape() const override;

	private:
		vec3 m_HalfExtents;
	};

	//--------------
	// SphereShape
	//
	class SphereShape final : public CollisionShape
	{
		RTTR_ENABLE(CollisionShape)
		RTTR_REGISTRATION_FRIEND

	public:
		SphereShape() : CollisionShape() {}
		~SphereShape() = default;

		btCollisionShape* MakeBulletCollisionShape() const override;

	private:
		float m_Radius = 1.f;
	};

	// construct destruct
	//--------------------
	RigidBodyComponentDesc() : ComponentDescriptor<RigidBodyComponent>() {}
	~RigidBodyComponentDesc() = default;

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
