#include "stdafx.h"
#include "TransformComponent.h"

#include <EtCore/Reflection/Registration.h>


namespace et {
namespace fw {


// reflection
//------------

RTTR_REGISTRATION
{
	BEGIN_REGISTER_CLASS(TransformComponent, "transform component")
		.property("position", &TransformComponent::GetPosition, &TransformComponent::SetPosition)
		.property("rotation", &TransformComponent::GetRotation, &TransformComponent::SetRotation)
		.property("scale", &TransformComponent::GetScale, &TransformComponent::SetScale)
	END_REGISTER_CLASS_POLYMORPHIC(TransformComponent, I_ComponentDescriptor);
}

ECS_REGISTER_COMPONENT(TransformComponent);


//=====================
// Transform Component 
//=====================


//-------------------------------
// TransformComponent::Translate
//
void TransformComponent::Translate(vec3 const& translation)
{
	m_TransformChanged |= E_TransformChanged::Translation;
	m_Position = m_Position + translation;
}

//---------------------------------
// TransformComponent::SetPosition
//
void TransformComponent::SetPosition(vec3 const& position)
{
	m_TransformChanged |= E_TransformChanged::Translation;
	m_Position = position;
}

//--------------------------------------
// TransformComponent::SetWorldPosition
//
void TransformComponent::SetWorldPosition(vec3 const& position)
{
	m_TransformChanged |= E_TransformChanged::Translation;
	m_Position = (m_WorldPosition - m_Position) + position;
}

//---------------------------------
// TransformComponent::RotateEuler
//
void TransformComponent::RotateEuler(vec3 const& eulerAngles)
{
	m_TransformChanged |= E_TransformChanged::Rotation;

	m_Rotation = quat(eulerAngles);
}

//----------------------------
// TransformComponent::Rotate
//
void TransformComponent::Rotate(quat const& rotation)
{
	m_TransformChanged |= E_TransformChanged::Rotation;

	m_Rotation = m_Rotation * rotation;
}

//---------------------------------
// TransformComponent::SetRotation
//
void TransformComponent::SetRotation(quat const& rotation)
{
	m_TransformChanged |= E_TransformChanged::Rotation;

	m_Rotation = rotation;
}

//--------------------------------------
// TransformComponent::SetWorldRotation
//
void TransformComponent::SetWorldRotation(quat const& rotation)
{
	m_TransformChanged |= E_TransformChanged::Rotation;

	m_Rotation = (m_WorldRotation * math::inverse(m_Rotation)) * rotation; // should use inverse safe ? is quaterion always normalized ?
}

//------------------------------
// TransformComponent::SetScale
//
void TransformComponent::SetScale(vec3 const& scale)
{
	m_TransformChanged |= E_TransformChanged::Scale;
	m_Scale = scale;
}


} // namespace fw
} // namespace et
