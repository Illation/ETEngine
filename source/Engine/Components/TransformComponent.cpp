#include "stdafx.hpp"

#include "TransformComponent.hpp"
#include "LightComponent.hpp"
#include "../SceneGraph/Entity.hpp"


TransformComponent::TransformComponent()
{
	m_IsTransformChanged = TransformChanged::NONE;
}


TransformComponent::~TransformComponent()
{
}

void TransformComponent::Initialize()
{
	UpdateTransforms();
}

void TransformComponent::Update()
{
	UpdateTransforms();
}

void TransformComponent::UpdateTransforms()
{
	if (m_IsTransformChanged & TransformChanged::NONE)
		return;

	if (m_IsTransformChanged & TransformChanged::TRANSLATION | TransformChanged::ROTATION)
	{
		auto ligComp = m_pEntity->GetComponent<LightComponent>();
		if (!(ligComp == nullptr))
		{
			ligComp->m_PositionUpdated = true;
		}
	}

	//Calculate World Matrix
	//**********************
	m_World = etm::scale(m_Scale)*etm::rotate(m_Rotation)*etm::translate(m_Position);

	Entity* parent = m_pEntity->GetParent();
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
	
	//Get World Transform
	//#todo there is an issue where glm decomposition does not have a left handed version, 
	//thus the resulting world rotation is mangled as if its input had been a right handed rotation
	//this problem has been overlooked for a large part of the engines development, resulting in code that is adapted to work with this bug
	//the correct solution would be to not use decomposition here at all, but because of the adapted code the correct solution causes incorrect behaviour
	//therefore we temporarily override the correct result with the faulty decomposited rotation
	//glm::quat rot;
	//if (glm::decompose(m_World, glm::vec3(), rot, glm::vec3(), glm::vec3(), glm::vec4()))
	//{
	//	m_WorldRotation = rot;
	//}

	m_Forward = m_WorldRotation*vec3::FORWARD;
	m_Right = m_WorldRotation*vec3::RIGHT;
	m_Up = etm::cross(m_Forward, m_Right);

	m_IsTransformChanged = TransformChanged::NONE;
}

void TransformComponent::Draw()
{
}
void TransformComponent::DrawForward()
{
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