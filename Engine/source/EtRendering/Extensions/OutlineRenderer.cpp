#include "stdafx.h"
#include "OutlineRenderer.h"

#include <EtCore/Content/ResourceManager.h>

#include <EtRHI/GraphicsTypes/TextureData.h>
#include <EtRHI/GraphicsTypes/Shader.h>
#include <EtRHI/Util/PrimitiveRenderer.h>

#include <EtRendering/GraphicsTypes/FrameBuffer.h>
#include <EtRendering/MaterialSystem/MaterialData.h>
#include <EtRendering/GlobalRenderingSystems/GlobalRenderingSystems.h>
#include <EtRendering/SceneStructure/RenderScene.h>
#include <EtRendering/SceneRendering/ShadedSceneRenderer.h>
#include <EtRendering/SceneRendering/Gbuffer.h>


namespace et {
namespace render {


//====================
// Outline Renderer
//====================


//---------------------------------
// OutlineRenderer::d-tor
//
OutlineRenderer::~OutlineRenderer()
{
	if (m_IsInitialized)
	{
		Deinit();
	}
}

//---------------------------------
// OutlineRenderer::Init
//
void OutlineRenderer::Init(Ptr<render::T_RenderEventDispatcher> const eventDispatcher)
{
	m_SobelShader = core::ResourceManager::Instance()->GetAssetData<rhi::ShaderData>(core::HashString("Shaders/PostSobel.glsl"));

	CreateRenderTarget();

	m_VPCallbackId = rhi::Viewport::GetCurrentViewport()->GetEventDispatcher().Register(rhi::E_ViewportEvent::VP_Resized, rhi::T_ViewportEventCallback(
		[this](rhi::T_ViewportEventFlags const, rhi::ViewportEventData const* const) -> void
		{
			OnWindowResize();
		}));

	m_EventDispatcher = eventDispatcher;
	m_CallbackId = m_EventDispatcher->Register(render::E_RenderEvent::RE_RenderOverlay, render::T_RenderEventCallback(
		[this](render::T_RenderEventFlags const flags, render::RenderEventData const* const evnt) -> void
		{
			ET_UNUSED(flags);

			if (evnt->renderer->GetType() == rttr::type::get<render::ShadedSceneRenderer>())
			{
				render::ShadedSceneRenderer const* const renderer = static_cast<render::ShadedSceneRenderer const*>(evnt->renderer);
				render::I_SceneExtension const* const ext = renderer->GetScene()->GetExtension(OutlineExtension::s_ExtensionId);
				if (ext == nullptr)
				{
					ET_TRACE_V(ET_CTX_RENDER, "render scene does not have an outline extension");
					return;
				}

				OutlineExtension const* const outlineExt = static_cast<OutlineExtension const*>(ext);
				Draw(evnt->targetFb, *outlineExt, renderer->GetScene()->GetNodes(), renderer->GetCamera(), renderer->GetGBuffer());
			}
			else
			{
				ET_ASSERT(true, "Cannot retrieve outline info from unhandled renderer!");
			}
		}));

	m_IsInitialized = true;
}

//---------------------------------
// OutlineRenderer::Deinit
//
void OutlineRenderer::Deinit()
{
	DestroyRenderTarget();

	m_SobelShader = nullptr;

	if (m_VPCallbackId != rhi::T_ViewportEventDispatcher::INVALID_ID)
	{
		rhi::Viewport::GetCurrentViewport()->GetEventDispatcher().Unregister(m_VPCallbackId);
	}

	if (m_EventDispatcher != nullptr)
	{
		m_EventDispatcher->Unregister(m_CallbackId);
		m_EventDispatcher = nullptr;
	}

	m_IsInitialized = false;
}

//---------------------------------
// OutlineRenderer::GenerateRenderTarget
//
void OutlineRenderer::CreateRenderTarget()
{
	rhi::I_RenderDevice* const device = rhi::ContextHolder::GetRenderDevice();
	ivec2 const dim = rhi::Viewport::GetCurrentViewport()->GetDimensions();

	rhi::TextureParameters params(false);
	params.minFilter = rhi::E_TextureFilterMode::Linear;
	params.magFilter = rhi::E_TextureFilterMode::Linear;
	params.wrapS = rhi::E_TextureWrapMode::ClampToBorder;
	params.wrapT = rhi::E_TextureWrapMode::ClampToBorder;
	params.borderColor = vec4(vec3(0.f), 1.f);

	//Generate texture and fbo and rbo as initial postprocessing target
	device->GenFramebuffers(1, &m_DrawTarget);
	device->BindFramebuffer(m_DrawTarget);
	m_DrawTex = new rhi::TextureData(rhi::E_ColorFormat::RGB16f, dim);
	m_DrawTex->AllocateStorage();
	m_DrawTex->SetParameters(params);
	device->LinkTextureToFbo2D(0, m_DrawTex->GetLocation(), 0);

	device->GenRenderBuffers(1, &m_DrawDepth);
	device->BindRenderbuffer(m_DrawDepth);
	device->SetRenderbufferStorage(rhi::E_RenderBufferFormat::Depth24, dim);
	device->LinkRenderbufferToFbo(rhi::E_RenderBufferFormat::Depth24, m_DrawDepth);

	device->BindFramebuffer(0);
}

//---------------------------------
// OutlineRenderer::DestroyRenderTarget
//
void OutlineRenderer::DestroyRenderTarget()
{
	rhi::I_RenderDevice* const device = rhi::ContextHolder::GetRenderDevice();

	device->DeleteRenderBuffers(1, &m_DrawDepth);
	SafeDelete(m_DrawTex);
	device->DeleteFramebuffers(1, &m_DrawTarget);
}

//---------------------------------
// OutlineRenderer::Draw
//
void OutlineRenderer::Draw(rhi::T_FbLoc const targetFb, 
	OutlineExtension const& outlines, 
	core::slot_map<mat4> const& nodes, 
	Camera const& cam, 
	Gbuffer const& gbuffer)
{
	if (outlines.GetOutlineLists().empty())
	{
		return;
	}

	rhi::I_RenderDevice* const device = rhi::ContextHolder::GetRenderDevice();
	ivec2 const dim = rhi::Viewport::GetCurrentViewport()->GetDimensions();

	device->SetViewport(ivec2(0), dim);

	// draw the shapes as colors to the intermediate rendertarget
	//------------------------------------------------------------
	device->BindFramebuffer(m_DrawTarget);

	device->SetClearColor(vec4(vec3(0.f), 1.f));
	device->Clear(rhi::E_ClearFlag::CF_Color | rhi::E_ClearFlag::CF_Depth);

	render::Material const* const mat = RenderingSystems::Instance()->GetColorMaterial();
	rhi::ShaderData const* const shader = mat->GetShader();

	device->SetShader(shader);
	shader->Upload("uViewSize"_hash, math::vecCast<float>(dim));

	shader->Upload("uOcclusionFactor"_hash, 0.15f);

	shader->Upload("uTexGBufferA"_hash, static_cast<rhi::TextureData const*>(gbuffer.GetTextures()[0]));

	device->SetDepthEnabled(true); 

	for (OutlineExtension::OutlineList const& list : outlines.GetOutlineLists())
	{
		shader->Upload("uColor"_hash, list.color);

		for (render::MaterialCollection::Mesh const& mesh : list.meshes)
		{
			device->BindVertexArray(mesh.m_VAO);
			for (render::T_NodeId const node : mesh.m_Instances)
			{
				// #todo: collect a list of transforms and draw this instanced
				mat4 const& transform = nodes[node];
				math::Sphere instSphere = math::Sphere((transform * vec4(mesh.m_BoundingVolume.pos, 1.f)).xyz,
					math::length(math::decomposeScale(transform)) * mesh.m_BoundingVolume.radius);

				if (cam.GetFrustum().ContainsSphere(instSphere) != VolumeCheck::OUTSIDE)
				{
					shader->Upload("model"_hash, transform);
					device->DrawElements(rhi::E_DrawMode::Triangles, mesh.m_IndexCount, mesh.m_IndexDataType, 0);
				}
			}
		}
	}

	// apply a sobel shader to the colored shapes and render it to the target framebuffer
	//------------------------------------------------------------------------------------

	device->BindFramebuffer(targetFb);

	device->SetDepthEnabled(false);
	device->SetBlendEnabled(true);
	device->SetBlendEquation(rhi::E_BlendEquation::Add);
	device->SetBlendFunction(rhi::E_BlendFactor::One, rhi::E_BlendFactor::One);

	device->SetShader(m_SobelShader.get());

	m_SobelShader->Upload("inColorTex"_hash, static_cast<rhi::TextureData const*>(m_DrawTex));

	rhi::PrimitiveRenderer::Instance().Draw<rhi::primitives::Quad>();

	device->SetBlendEnabled(false);
}

//---------------------------------
// OutlineRenderer::OnWindowResize
//
void OutlineRenderer::OnWindowResize()
{
	DestroyRenderTarget();
	CreateRenderTarget();
}


} // namespace render
} // namespace et
