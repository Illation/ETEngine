#include "stdafx.h"
#include "Light.h"

#include <Engine/GraphicsHelper/LightVolume.h>
#include <Engine/GraphicsHelper/ShadowRenderer.h>


void PointLight::DrawVolume(TransformComponent* pTransform)
{
	vec3 col = color*brightness;
	PointLightVolume::GetInstance()->Draw(pTransform->GetPosition(), radius, col);
}

void DirectionalLight::DrawVolume(TransformComponent* pTransform)
{
	vec3 col = color*brightness;
	if (IsShadowEnabled())
	{
		DirectLightVolume::GetInstance()->DrawShadowed(pTransform->GetForward(), col, m_pShadowData);
	}
	else
	{
		DirectLightVolume::GetInstance()->Draw(pTransform->GetForward(), col);
	}
}

void DirectionalLight::SetShadowEnabled(bool enabled)
{
	if (enabled)
	{
		if(!IsShadowEnabled())
			m_pShadowData = new DirectionalShadowData(ivec2(1024, 1024)*8);
	}
	else
	{
		SafeDelete(m_pShadowData);
		m_pShadowData = nullptr;
	}
}

void DirectionalLight::GenerateShadow(TransformComponent* pTransform)
{
	if(IsShadowEnabled())ShadowRenderer::GetInstance()->MapDirectional(pTransform, m_pShadowData);
}