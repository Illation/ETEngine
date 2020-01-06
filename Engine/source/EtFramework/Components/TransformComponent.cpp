#include "stdafx.h"
#include "TransformComponent.h"

#include "LightComponent.h"
#include "RigidBodyComponent.h"

#include <rttr/registration>

#include <EtFramework/SceneGraph/Entity.h>
#include <EtFramework/SceneGraph/SceneManager.h>


// deprecated
//------------

TransformComponent::TransformComponent()
{
	m_IsTransformChanged = TransformChanged::NONE;
}

void TransformComponent::Init()
{
	m_NodeId = SceneManager::GetInstance()->GetRenderScene().AddNode(m_World);
	UpdateTransforms();
}

void TransformComponent::Deinit()
{
	SceneManager::GetInstance()->GetRenderScene().RemoveNode(m_NodeId);
}

void TransformComponent::Update()
{
	UpdateTransforms();
}

void TransformComponent::UpdateTransforms()
{
	//Rigid body handling
	RigidBodyComponent* const rigidbody = GetEntity()->GetComponent<RigidBodyComponent>();
	if (rigidbody != nullptr)
	{
		if (m_IsTransformChanged & TransformChanged::TRANSLATION)
			rigidbody->SetPosition(m_Position);
		else
			m_Position = rigidbody->GetPosition();

		if (m_IsTransformChanged & TransformChanged::ROTATION)
			rigidbody->SetRotation(m_Rotation);
		else
			m_Rotation = rigidbody->GetRotation();
	}
	else if (m_IsTransformChanged & TransformChanged::NONE)
	{
		return;
	}

	//Calculate World Matrix
	//**********************
	m_World = etm::scale(m_Scale)*etm::rotate(m_Rotation)*etm::translate(m_Position);

	Entity* parent = GetEntity()->GetParent();
	if (parent)
	{
		mat4 parentWorld = parent->GetTransform()->m_World;
		m_World = m_World * parentWorld;

		m_WorldPosition = (parentWorld * vec4(m_Position, 0)).xyz;
		m_WorldRotation = parent->GetTransform()->GetWorldRotation() * m_WorldRotation; //maybe flip
		m_WorldScale = m_Scale * parent->GetTransform()->GetWorldScale();
	}
	else
	{
		m_WorldPosition = m_Position;
		m_WorldRotation = m_Rotation;
		m_WorldScale = m_Scale;
	}

	m_Forward = m_WorldRotation*vec3::FORWARD;
	m_Right = m_WorldRotation*vec3::RIGHT;
	m_Up = etm::cross(m_Forward, m_Right);

	SceneManager::GetInstance()->GetRenderScene().UpdateNode(m_NodeId, m_World);
	m_IsTransformChanged = TransformChanged::NONE;
}

void TransformComponent::Translate(float x, float y, float z)
{
	Translate( vec3( x, y, z ) );
}
void TransformComponent::Translate(const vec3& translation )
{
	m_IsTransformChanged |= TransformChanged::TRANSLATION;
	m_Position = m_Position + translation;
}
void TransformComponent::SetPosition(float x, float y, float z)
{
	SetPosition( vec3( x, y, z ) );
}
void TransformComponent::SetPosition(const vec3& position)
{
	m_IsTransformChanged |= TransformChanged::TRANSLATION;
	m_Position = position;
}

void TransformComponent::RotateEuler(float x, float y, float z)
{
	RotateEuler(vec3(x, y, z));
}
void TransformComponent::RotateEuler(const vec3& eulerAngles)
{
	m_IsTransformChanged |= TransformChanged::ROTATION;

	m_Rotation = quat( eulerAngles );
}
void TransformComponent::Rotate(const quat& rotation)
{
	m_IsTransformChanged |= TransformChanged::ROTATION;

	m_Rotation = m_Rotation * rotation;
}

void TransformComponent::Scale(float x, float y, float z)
{
	Scale( vec3( x, y, z ) );
}
void TransformComponent::Scale(const vec3& scale)
{
	m_IsTransformChanged |= TransformChanged::SCALE;
	m_Scale = scale;
}


// reflection
//------------

RTTR_REGISTRATION
{
	using namespace rttr;

	registration::class_<fw::TransformComponent>("transform component");

	registration::class_<fw::TransformComponentDesc>("transform comp desc")
		.constructor<fw::TransformComponentDesc const&>()
		.constructor<>()(rttr::detail::as_object())
		.property("position", &fw::TransformComponentDesc::position)
		.property("rotation", &fw::TransformComponentDesc::rotation)
		.property("scale", &fw::TransformComponentDesc::scale);

	rttr::type::register_converter_func([](fw::TransformComponentDesc& descriptor, bool& ok) -> fw::I_ComponentDescriptor*
	{
		ok = true;
		return new fw::TransformComponentDesc(descriptor);
	});
}

// component registration
//------------------------

ECS_REGISTER_COMPONENT(fw::TransformComponent);


namespace fw {


//=====================
// Transform Component 
//=====================


//-------------------------------
// TransformComponent::Translate
//
void TransformComponent::Translate(float x, float y, float z)
{
	Translate(vec3(x, y, z));
}

//-------------------------------
// TransformComponent::Translate
//
void TransformComponent::Translate(const vec3& translation)
{
	m_TransformChanged |= E_TransformChanged::Translation;
	m_Position = m_Position + translation;
}

//---------------------------------
// TransformComponent::SetPosition
//
void TransformComponent::SetPosition(float x, float y, float z)
{
	SetPosition(vec3(x, y, z));
}

//---------------------------------
// TransformComponent::SetPosition
//
void TransformComponent::SetPosition(const vec3& position)
{
	m_TransformChanged |= E_TransformChanged::Translation;
	m_Position = position;
}

//---------------------------------
// TransformComponent::RotateEuler
//
void TransformComponent::RotateEuler(float x, float y, float z)
{
	RotateEuler(vec3(x, y, z));
}

//---------------------------------
// TransformComponent::RotateEuler
//
void TransformComponent::RotateEuler(const vec3& eulerAngles)
{
	m_TransformChanged |= E_TransformChanged::Rotation;

	m_Rotation = quat(eulerAngles);
}

//----------------------------
// TransformComponent::Rotate
//
void TransformComponent::Rotate(const quat& rotation)
{
	m_TransformChanged |= E_TransformChanged::Rotation;

	m_Rotation = m_Rotation * rotation;
}

//------------------------------
// TransformComponent::SetScale
//
void TransformComponent::SetScale(float x, float y, float z)
{
	SetScale(vec3(x, y, z));
}

//------------------------------
// TransformComponent::SetScale
//
void TransformComponent::SetScale(const vec3& scale)
{
	m_TransformChanged |= E_TransformChanged::Scale;
	m_Scale = scale;
}


//================================
// Transform Component Descriptor
//================================


//----------------------------------
// TransformComponentDesc::MakeData
//
// Create a transform component from a descriptor
//
TransformComponent* TransformComponentDesc::MakeData()
{
	TransformComponent* const ret = new TransformComponent();

	ret->SetPosition(position);
	ret->SetRotation(rotation);
	ret->SetScale(scale);

	return ret;
}


} // namespace fw
