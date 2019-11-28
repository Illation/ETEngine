#include "stdafx.h"
#include "ScreenSpaceReflections.h"

#include "ShadedSceneRenderer.h"
#include "Gbuffer.h"
#include "PostProcessingRenderer.h"

#include <EtCore/Content/ResourceManager.h>
#include <EtCore/Helper/Commands.h>

#include <EtRendering/GraphicsTypes/TextureData.h>
#include <EtRendering/GraphicsTypes/Shader.h>
#include <EtRendering/GlobalRenderingSystems/GlobalRenderingSystems.h>


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
	//Render Buffer for depth and stencil
	api->GenRenderBuffers(1, &m_CollectRBO);
	api->BindRenderbuffer(m_CollectRBO);
	api->SetRenderbufferStorage(E_RenderBufferFormat::Depth24, dim);
	api->LinkRenderbufferToFbo(E_RenderBufferFormat::Depth24, m_CollectRBO);
	api->LinkTextureToFbo2D(0, m_CollectTex->GetHandle(), 0);
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
	auto gbufferTex = render::ShadedSceneRenderer::GetCurrent()->GetGBuffer().GetTextures();
	for (uint32 i = 0; i < (uint32)gbufferTex.size(); i++)
	{
		api->LazyBindTexture(i, E_TextureType::Texture2D, gbufferTex[i]->GetHandle());
	}
	m_pShader->Upload("uFinalImage"_hash, 3);
	api->LazyBindTexture(3, E_TextureType::Texture2D, m_CollectTex->GetHandle());
	//for position reconstruction
	BaseContext* const context = ContextManager::GetInstance()->GetActiveContext();
	if (context != nullptr)
	{
		m_pShader->Upload("K"_hash, sinf(context->time->GetTime()) * 20 + 25);
	}

	RenderingSystems::Instance()->GetPrimitiveRenderer().Draw<primitives::Quad>();
}