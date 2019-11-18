#include "stdafx.h"
#include "CameraComponent.h"

#include <EtRendering/GraphicsTypes/Camera.h>
#include <EtRendering/GraphicsContext/Viewport.h>


CameraComponent::CameraComponent() = default;

void CameraComponent::PopulateCamera(Camera& target, Viewport const& viewport) const
{
	TransformComponent const* const tfComp = GetTransform();
	target.SetTransformation(tfComp->GetPosition(), tfComp->GetForward(), tfComp->GetUp(), true);

	target.SetIsPerspective(m_IsPerspective, true);
	target.SetFieldOfView(m_FieldOfView, true);
	target.SetSize(m_Size, true);
	target.SetClippingPlanes(m_NearPlane, m_FarPlane, true);
	target.SetViewport(&viewport, true);

	target.Recalculate();
}
