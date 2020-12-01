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
void TransformComponent::Translate(const vec3& translation)
{
	m_TransformChanged |= E_TransformChanged::Translation;
	m_Position = m_Position + translation;
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

//---------------------------------
// TransformComponent::SetRotation
//
void TransformComponent::SetRotation(const quat& rotation)
{
	m_TransformChanged |= E_TransformChanged::Rotation;

	m_Rotation = rotation;
}

//------------------------------
// TransformComponent::SetScale
//
void TransformComponent::SetScale(const vec3& scale)
{
	m_TransformChanged |= E_TransformChanged::Scale;
	m_Scale = scale;
}


} // namespace fw
} // namespace et
