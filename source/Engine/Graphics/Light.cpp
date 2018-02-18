#include "stdafx.hpp"
#include "Light.hpp"

#include "../GraphicsHelper/LightVolume.hpp"
#include "../GraphicsHelper/ShadowRenderer.hpp"

void PointLight::UploadVariables(GLuint program, TransformComponent* comp, uint32 index)
{
	std::string idxStr = std::to_string(index);
	std::string ligStr = "pointLights[";

	vec3 pos = comp->GetPosition();
	vec3 col = color*brightness;

	glUniform3f(glGetUniformLocation(program,
		(ligStr + idxStr + "].Position").c_str()), pos.x, pos.y, pos.z);
	glUniform3f(glGetUniformLocation(program,
		(ligStr + idxStr + "].Color").c_str()), col.x, col.y, col.z);
	glUniform1f(glGetUniformLocation(program, 
		(ligStr + idxStr + "].Radius").c_str()), radius);
}
void PointLight::DrawVolume(TransformComponent* pTransform)
{
	vec3 col = color*brightness;
	PointLightVolume::GetInstance()->Draw(pTransform->GetPosition(), radius, col);
}


void DirectionalLight::UploadVariables(GLuint program, TransformComponent* comp, uint32 index)
{
	std::string idxStr = std::to_string(index);
	std::string ligStr = "dirLights[";

	vec3 direction = comp->GetForward();
	vec3 col = color*brightness;

	glUniform3f(glGetUniformLocation(program,
		(ligStr + idxStr + "].Direction").c_str()), direction.x, direction.y, direction.z);
	glUniform3f(glGetUniformLocation(program,
		(ligStr + idxStr + "].Color").c_str()), col.x, col.y, col.z);
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