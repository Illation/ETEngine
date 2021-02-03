#include "stdafx.h"
#include "ScreenSpaceReflections.h"

#include "ShadedSceneRenderer.h"
#include "Gbuffer.h"
#include "PostProcessingRenderer.h"

#include <EtCore/Content/ResourceManager.h>

#include <EtRendering/GraphicsTypes/TextureData.h>
#include <EtRendering/GraphicsTypes/Shader.h>
#include <EtRendering/GlobalRenderingSystems/GlobalRenderingSystems.h>


namespace et {
namespace render {


ScreenSpaceReflections::~ScreenSpaceReflections()
{
	I_GraphicsContextApi* const api = ContextHolder::GetRenderContext();

	api->DeleteRenderBuffers(1, &m_CollectRBO);
	SafeDelete(m_CollectTex);
	api->DeleteFramebuffers(1, &m_CollectFBO);
}

void ScreenSpaceReflections::Initialize()
{
	I_GraphicsContextApi* const api = ContextHolder::GetRenderContext();
	ivec2 const dim = Viewport::GetCurrentViewport()->GetDimensions();

	m_pShader = core::ResourceManager::Instance()->GetAssetData<ShaderData>(core::HashString("Shaders/PostScreenSpaceReflections.glsl"));

	TextureParameters params(false);
	params.minFilter = E_TextureFilterMode::Linear;
	params.magFilter = E_TextureFilterMode::Linear;
	params.wrapS = E_TextureWrapMode::ClampToEdge;
	params.wrapT = E_TextureWrapMode::ClampToEdge;

	//Generate texture and fbo and rbo as initial postprocessing target
	api->GenFramebuffers(1, &m_CollectFBO);
	api->BindFramebuffer(m_CollectFBO);
	m_CollectTex = new TextureData(E_ColorFormat::RGB16f, dim);
	m_CollectTex->AllocateStorage();
	m_CollectTex->SetParameters(params);

	//Render Buffer for depth and stencil
	api->GenRenderBuffers(1, &m_CollectRBO);
	api->BindRenderbuffer(m_CollectRBO);
	api->SetRenderbufferStorage(E_RenderBufferFormat::Depth24, dim);
	api->LinkRenderbufferToFbo(E_RenderBufferFormat::Depth24, m_CollectRBO);
	api->LinkTextureToFbo2D(0, m_CollectTex->GetLocation(), 0);

	// cleanup
	api->BindRenderbuffer(0);
	api->BindFramebuffer(0);
}

void ScreenSpaceReflections::EnableInput()
{
	ContextHolder::GetRenderContext()->BindFramebuffer(m_CollectFBO);
}

void ScreenSpaceReflections::Draw()
{
	I_GraphicsContextApi* const api = ContextHolder::GetRenderContext();

	api->SetShader(m_pShader.get());

	m_pShader->Upload("uFinalImage"_hash, static_cast<TextureData const*>(m_CollectTex));

	//for position reconstruction
	core::BaseContext* const context = core::ContextManager::GetInstance()->GetActiveContext();
	if (context != nullptr)
	{
		m_pShader->Upload("K"_hash, sinf(context->time->GetTime()) * 20 + 25);
	}

	RenderingSystems::Instance()->GetPrimitiveRenderer().Draw<primitives::Quad>();
}

} // namespace render
} // namespace et
