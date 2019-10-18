#include "stdafx.h"
#include "ScreenSpaceReflections.h"

#include "SceneRenderer.h"
#include "Gbuffer.h"
#include "PostProcessingRenderer.h"

#include <EtCore/Content/ResourceManager.h>
#include <EtCore/Helper/Commands.h>

#include <Engine/Graphics/TextureData.h>
#include <Engine/Graphics/Shader.h>
#include <Engine/GlobalRenderingSystems/GlobalRenderingSystems.h>
#include <Engine/SceneRendering/SceneRenderer.h>


ScreenSpaceReflections::ScreenSpaceReflections()
{

}

ScreenSpaceReflections::~ScreenSpaceReflections()
{
	I_GraphicsApiContext* const api = Viewport::GetCurrentApiContext();

	api->DeleteRenderBuffers(1, &m_CollectRBO);
	SafeDelete(m_CollectTex);
	api->DeleteFramebuffers(1, &m_CollectFBO);
}

void ScreenSpaceReflections::Initialize()
{
	I_GraphicsApiContext* const api = Viewport::GetCurrentApiContext();
	ivec2 const dim = Viewport::GetCurrentViewport()->GetDimensions();

	m_pShader = ResourceManager::Instance()->GetAssetData<ShaderData>("PostScreenSpaceReflections.glsl"_hash);

	TextureParameters params(false);
	params.minFilter = E_TextureFilterMode::Linear;
	params.magFilter = E_TextureFilterMode::Linear;
	params.wrapS = E_TextureWrapMode::ClampToEdge;
	params.wrapT = E_TextureWrapMode::ClampToEdge;

	//Generate texture and fbo and rbo as initial postprocessing target
	api->GenFramebuffers(1, &m_CollectFBO);
	api->BindFramebuffer(m_CollectFBO);
	m_CollectTex = new TextureData(dim, E_ColorFormat::RGB16f, E_ColorFormat::RGB, E_DataType::Float);
	m_CollectTex->Build();
	m_CollectTex->SetParameters(params);
	api->LinkTextureToFbo2D(0, m_CollectTex->GetHandle(), 0);
	//Render Buffer for depth and stencil
	api->GenRenderBuffers(1, &m_CollectRBO);
	api->BindRenderbuffer(m_CollectRBO);
	api->SetRenderbufferStorage(E_RenderBufferFormat::Depth24_Stencil8, dim);
	api->LinkRenderbufferToFbo(E_RenderBufferFormat::Depth24_Stencil8, m_CollectRBO);
}

void ScreenSpaceReflections::EnableInput()
{
	Viewport::GetCurrentApiContext()->BindFramebuffer(m_CollectFBO);
}

void ScreenSpaceReflections::Draw()
{
	I_GraphicsApiContext* const api = Viewport::GetCurrentApiContext();

	api->SetShader(m_pShader.get());

	m_pShader->Upload("texGBufferA"_hash, 0);
	m_pShader->Upload("texGBufferB"_hash, 1);
	m_pShader->Upload("texGBufferC"_hash, 2);
	auto gbufferTex = SceneRenderer::GetCurrent()->GetGBuffer()->GetTextures();
	for (uint32 i = 0; i < (uint32)gbufferTex.size(); i++)
	{
		api->LazyBindTexture(i, E_TextureType::Texture2D, gbufferTex[i]->GetHandle());
	}
	m_pShader->Upload("uFinalImage"_hash, 3);
	api->LazyBindTexture(3, E_TextureType::Texture2D, m_CollectTex->GetHandle());
	//for position reconstruction
	m_pShader->Upload("K"_hash, sinf(TIME->GetTime()) * 20 + 25);

	Camera const& cam = SceneRenderer::GetCurrent()->GetCamera();

	m_pShader->Upload("projectionA"_hash, cam.GetDepthProjA());
	m_pShader->Upload("projectionB"_hash, cam.GetDepthProjB());
	m_pShader->Upload("viewProjInv"_hash, cam.GetStatViewProjInv());
	m_pShader->Upload("projection"_hash, cam.GetViewProj());
	m_pShader->Upload("camPos"_hash, cam.GetPosition());

	RenderingSystems::Instance()->GetPrimitiveRenderer().Draw<primitives::Quad>();
}