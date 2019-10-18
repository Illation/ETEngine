#include "stdafx.h"
#include "CameraComponent.h"

#include <Engine/Graphics/Camera.h>


CameraComponent::CameraComponent() = default;

void CameraComponent::PopulateCamera(Camera& target) const
{
	TransformComponent const* const tfComp = GetTransform();
	target.SetTransformation(tfComp->GetPosition(), tfComp->GetForward(), tfComp->GetUp(), true);

	target.SetIsPerspective(m_IsPerspective, true);
	target.SetFieldOfView(m_FieldOfView, true);
	target.SetSize(m_Size, true);
	target.SetClippingPlanes(m_NearPlane, m_FarPlane, true);

	target.Recalculate();
}
