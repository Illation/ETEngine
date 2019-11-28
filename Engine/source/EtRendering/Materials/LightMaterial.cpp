#include "stdafx.h"
#include "LightMaterial.h"

#include <EtRendering/GraphicsTypes/TextureData.h>
#include <EtRendering/GraphicsTypes/Shader.h>
#include <EtRendering/GraphicsTypes/FrameBuffer.h>
#include <EtRendering/SceneRendering/Gbuffer.h>
#include <EtRendering/SceneRendering/ShadedSceneRenderer.h>


LightMaterial::LightMaterial(vec3 col)
	: Material("Shaders/FwdLightPointShader.glsl")
	, m_Color(col)
{
	m_DrawForward = true;
}

void LightMaterial::SetLight(vec3 pos, vec3 color, float radius)
{
	m_Position = pos;
	m_Color = color;
	m_Radius = radius;
}

void LightMaterial::UploadDerivedVariables()
{
	m_Shader->Upload("texGBufferA"_hash, 0);
	m_Shader->Upload("texGBufferB"_hash, 1);
	m_Shader->Upload("texGBufferC"_hash, 2);
	auto gbufferTex = render::ShadedSceneRenderer::GetCurrent()->GetGBuffer().GetTextures();
	for (uint32 i = 0; i < (uint32)gbufferTex.size(); i++)
	{
		Viewport::GetCurrentApiContext()->LazyBindTexture(i, E_TextureType::Texture2D, gbufferTex[i]->GetHandle());
	}

	m_Shader->Upload("Position"_hash, m_Position);
	m_Shader->Upload("Color"_hash, m_Color);
	m_Shader->Upload("Radius"_hash, m_Radius);
}