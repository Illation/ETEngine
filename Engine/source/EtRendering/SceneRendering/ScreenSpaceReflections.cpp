#include "stdafx.h"
#include "ScreenSpaceReflections.h"

#include "ShadedSceneRenderer.h"
#include "Gbuffer.h"
#include "PostProcessingRenderer.h"

#include <EtCore/Content/ResourceManager.h>

#include <EtRHI/GraphicsTypes/TextureData.h>
#include <EtRHI/GraphicsTypes/Shader.h>
#include <EtRHI/Util/PrimitiveRenderer.h>


namespace et {
namespace render {


ScreenSpaceReflections::~ScreenSpaceReflections()
{
	rhi::I_RenderDevice* const device = rhi::ContextHolder::GetRenderDevice();

	device->DeleteRenderBuffers(1, &m_CollectRBO);
	SafeDelete(m_CollectTex);
	device->DeleteFramebuffers(1, &m_CollectFBO);
}

void ScreenSpaceReflections::Initialize()
{
	rhi::I_RenderDevice* const device = rhi::ContextHolder::GetRenderDevice();
	ivec2 const dim = rhi::Viewport::GetCurrentViewport()->GetDimensions();

	m_pShader = core::ResourceManager::Instance()->GetAssetData<rhi::ShaderData>(core::HashString("Shaders/PostScreenSpaceReflections.glsl"));

	rhi::TextureParameters params(false);
	params.minFilter = rhi::E_TextureFilterMode::Linear;
	params.magFilter = rhi::E_TextureFilterMode::Linear;
	params.wrapS = rhi::E_TextureWrapMode::ClampToEdge;
	params.wrapT = rhi::E_TextureWrapMode::ClampToEdge;

	//Generate texture and fbo and rbo as initial postprocessing target
	device->GenFramebuffers(1, &m_CollectFBO);
	device->BindFramebuffer(m_CollectFBO);
	m_CollectTex = new rhi::TextureData(rhi::E_ColorFormat::RGB16f, dim);
	m_CollectTex->AllocateStorage();
	m_CollectTex->SetParameters(params);

	//Render Buffer for depth and stencil
	device->GenRenderBuffers(1, &m_CollectRBO);
	device->BindRenderbuffer(m_CollectRBO);
	device->SetRenderbufferStorage(rhi::E_RenderBufferFormat::Depth24, dim);
	device->LinkRenderbufferToFbo(rhi::E_RenderBufferFormat::Depth24, m_CollectRBO);
	device->LinkTextureToFbo2D(0, m_CollectTex->GetLocation(), 0);

	// cleanup
	device->BindRenderbuffer(0);
	device->BindFramebuffer(0);
}

void ScreenSpaceReflections::EnableInput()
{
	rhi::ContextHolder::GetRenderDevice()->BindFramebuffer(m_CollectFBO);
}

void ScreenSpaceReflections::Draw()
{
	rhi::I_RenderDevice* const device = rhi::ContextHolder::GetRenderDevice();

	device->SetShader(m_pShader.get());

	m_pShader->Upload("uFinalImage"_hash, static_cast<rhi::TextureData const*>(m_CollectTex));

	//for position reconstruction
	core::BaseContext* const context = core::ContextManager::GetInstance()->GetActiveContext();
	if (context != nullptr)
	{
		m_pShader->Upload("K"_hash, sinf(context->time->GetTime()) * 20 + 25);
	}

	rhi::PrimitiveRenderer::Instance().Draw<rhi::primitives::Quad>();
}

} // namespace render
} // namespace et
