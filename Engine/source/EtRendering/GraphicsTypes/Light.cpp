#include "stdafx.h"
#include "Light.h"

#include <EtRendering/GlobalRenderingSystems/GlobalRenderingSystems.h>
#include <EtRendering/SceneRendering/ShadowRenderer.h>


void PointLight::DrawVolume(TransformComponent* pTransform)
{
	vec3 col = color*brightness;
	RenderingSystems::Instance()->GetPointLightVolume().Draw(pTransform->GetPosition(), radius, col);
}

void DirectionalLight::DrawVolume(TransformComponent* pTransform)
{
	vec3 col = color*brightness;
	if (IsShadowEnabled())
	{
		RenderingSystems::Instance()->GetDirectLightVolume().DrawShadowed(pTransform->GetForward(), col, *m_pShadowData);
	}
	else
	{
		RenderingSystems::Instance()->GetDirectLightVolume().Draw(pTransform->GetForward(), col);
	}
}

void DirectionalLight::SetShadowEnabled(bool enabled)
{
	if (enabled)
	{
		if (!IsShadowEnabled())
		{
			m_pShadowData = new DirectionalShadowData(ivec2(1024, 1024)*8);
		}
	}
	else
	{
		SafeDelete(m_pShadowData);
		m_pShadowData = nullptr;
	}
}
