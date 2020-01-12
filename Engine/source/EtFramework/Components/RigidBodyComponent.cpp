#include "stdafx.h"
#include "RigidBodyComponent.h"

#include <btBulletDynamicsCommon.h>

#include <EtFramework/Physics/BulletETM.h>


// reflection
//------------

RTTR_REGISTRATION
{
	using namespace rttr;

	registration::class_<fw::RigidBodyComponent>("rigid body component");

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


//-----------------------------
// RigidBodyComponentDesc:: = 
//
RigidBodyComponentDesc& RigidBodyComponentDesc::operator=(RigidBodyComponentDesc const& other)
{
	isDynamic = other.isDynamic;
	mass = other.mass;

	delete shape;
	shape = nullptr;
	if (other.shape != nullptr)
	{
		shape = other.shape->Clone();
	}

	return *this;
}

//-------------------------------
// RigidBodyComponentDesc::c-tor
//
RigidBodyComponentDesc::RigidBodyComponentDesc(RigidBodyComponentDesc const& other)
{
	*this = other;
}

//-------------------------------
// RigidBodyComponentDesc::d-tor
//
RigidBodyComponentDesc::~RigidBodyComponentDesc()
{
	delete shape;
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
