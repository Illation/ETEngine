#include "stdafx.h"
#include "ScreenSpaceReflections.h"

#include "RenderPipeline.h"
#include "PrimitiveRenderer.h"

#include <glad/glad.h>

#include <EtCore/Content/ResourceManager.h>
#include <EtCore/Helper/Commands.h>

#include <Engine/Graphics/TextureData.h>
#include <Engine/Graphics/Shader.h>
#include <Engine/Framebuffers/Gbuffer.h>
#include <Engine/Framebuffers/PostProcessingRenderer.h>


ScreenSpaceReflections::ScreenSpaceReflections()
{

}

ScreenSpaceReflections::~ScreenSpaceReflections()
{
	GraphicsApiContext* const api = Viewport::GetCurrentApiContext();

	api->DeleteRenderBuffers(1, &m_CollectRBO);
	SafeDelete(m_CollectTex);
	api->DeleteFramebuffers(1, &m_CollectFBO);
}

void ScreenSpaceReflections::Initialize()
{
	GraphicsApiContext* const api = Viewport::GetCurrentApiContext();

	m_pShader = ResourceManager::Instance()->GetAssetData<ShaderData>("PostScreenSpaceReflections.glsl"_hash);

	Config::Settings::Window const& windowSettings = Config::GetInstance()->GetWindow();

	TextureParameters params(false);
	params.minFilter = E_TextureFilterMode::Linear;
	params.magFilter = E_TextureFilterMode::Linear;
	params.wrapS = E_TextureWrapMode::ClampToEdge;
	params.wrapT = E_TextureWrapMode::ClampToEdge;

	//Generate texture and fbo and rbo as initial postprocessing target
	api->GenFramebuffers(1, &m_CollectFBO);
	api->BindFramebuffer(m_CollectFBO);
	m_CollectTex = new TextureData(windowSettings.Width, windowSettings.Height, GL_RGB16F, GL_RGB, GL_FLOAT);
	m_CollectTex->Build();
	m_CollectTex->SetParameters(params);
	api->LinkTextureToFbo2D(0, m_CollectTex->GetHandle(), 0);
	//Render Buffer for depth and stencil
	api->GenRenderBuffers(1, &m_CollectRBO);
	api->BindRenderbuffer(m_CollectRBO);
	api->SetRenderbufferStorage(E_RenderBufferFormat::Depth24_Stencil8, windowSettings.Dimensions);
	api->LinkRenderbufferToFbo(E_RenderBufferFormat::Depth24_Stencil8, m_CollectRBO);
}

void ScreenSpaceReflections::EnableInput()
{
	Viewport::GetCurrentApiContext()->BindFramebuffer(m_CollectFBO);
}

void ScreenSpaceReflections::Draw()
{
	GraphicsApiContext* const api = Viewport::GetCurrentApiContext();

	api->SetShader(m_pShader.get());

	m_pShader->Upload("texGBufferA"_hash, 0);
	m_pShader->Upload("texGBufferB"_hash, 1);
	m_pShader->Upload("texGBufferC"_hash, 2);
	auto gbufferTex = RenderPipeline::GetInstance()->GetGBuffer()->GetTextures();
	for (uint32 i = 0; i < (uint32)gbufferTex.size(); i++)
	{
		api->LazyBindTexture(i, E_TextureType::Texture2D, gbufferTex[i]->GetHandle());
	}
	m_pShader->Upload("uFinalImage"_hash, 3);
	api->LazyBindTexture(3, E_TextureType::Texture2D, m_CollectTex->GetHandle());
	//for position reconstruction
	m_pShader->Upload("K"_hash, sinf(TIME->GetTime()) * 20 + 25);
	m_pShader->Upload("projectionA"_hash, CAMERA->GetDepthProjA());
	m_pShader->Upload("projectionB"_hash, CAMERA->GetDepthProjB());
	m_pShader->Upload("viewProjInv"_hash, CAMERA->GetStatViewProjInv());
	m_pShader->Upload("projection"_hash, CAMERA->GetViewProj());
	m_pShader->Upload("camPos"_hash, CAMERA->GetTransform()->GetPosition());

	PrimitiveRenderer::GetInstance()->Draw<primitives::Quad>();
}