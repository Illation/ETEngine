#include "stdafx.h"
#include "LightMaterial.h"

#include <Engine/Graphics/TextureData.h>
#include <Engine/Graphics/Shader.h>
#include <Engine/Graphics/FrameBuffer.h>
#include <Engine/Framebuffers/Gbuffer.h>
#include <Engine/GraphicsHelper/RenderPipeline.h>


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
	auto gbufferTex = RenderPipeline::GetInstance()->GetGBuffer()->GetTextures();
	for (uint32 i = 0; i < (uint32)gbufferTex.size(); i++)
	{
		Viewport::GetCurrentApiContext()->LazyBindTexture(i, GL_TEXTURE_2D, gbufferTex[i]->GetHandle());
	}
	//for position reconstruction
	m_Shader->Upload("projectionA"_hash, CAMERA->GetDepthProjA());
	m_Shader->Upload("projectionB"_hash, CAMERA->GetDepthProjB());
	m_Shader->Upload("viewProjInv"_hash, CAMERA->GetStatViewProjInv());
	m_Shader->Upload("camPos"_hash, CAMERA->GetTransform()->GetPosition());

	m_Shader->Upload("Position"_hash, m_Position);
	m_Shader->Upload("Color"_hash, m_Color);
	m_Shader->Upload("Radius"_hash, m_Radius);
}