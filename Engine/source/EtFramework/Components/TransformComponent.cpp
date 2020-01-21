#include "stdafx.h"
#include "TransformComponent.h"

#include <EtCore/Reflection/Registration.h>


namespace et {
namespace fw {


// reflection
//------------

RTTR_REGISTRATION
{
	rttr::registration::class_<TransformComponent>("transform component");

	BEGIN_REGISTER_POLYMORPHIC_CLASS(TransformComponentDesc, "transform comp desc")
		.property("position", &TransformComponentDesc::position)
		.property("rotation", &TransformComponentDesc::rotation)
		.property("scale", &TransformComponentDesc::scale)
	END_REGISTER_POLYMORPHIC_CLASS(TransformComponentDesc, I_ComponentDescriptor);
}

ECS_REGISTER_COMPONENT(TransformComponent);


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
} // namespace et
