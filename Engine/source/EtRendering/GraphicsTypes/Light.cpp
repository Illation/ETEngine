#include "stdafx.h"
#include "Light.h"

#include <EtRendering/SceneRendering/ShadowRenderer.h>


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
