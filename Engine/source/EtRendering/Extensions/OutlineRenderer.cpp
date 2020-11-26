#include "stdafx.h"
#include "OutlineRenderer.h"

#include <EtCore/Content/ResourceManager.h>

#include <EtRendering/GraphicsTypes/TextureData.h>
#include <EtRendering/GraphicsTypes/Shader.h>
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
void OutlineRenderer::Init(render::T_RenderEventDispatcher* const eventDispatcher)
{
	m_SobelShader = core::ResourceManager::Instance()->GetAssetData<ShaderData>(core::HashString("PostSobel.glsl"));

	CreateRenderTarget();

	m_VPCallbackId = Viewport::GetCurrentViewport()->GetEventDispatcher().Register(render::E_ViewportEvent::VP_Resized, render::T_ViewportEventCallback(
		[this](render::T_ViewportEventFlags const, render::ViewportEventData const* const) -> void
		{
			OnWindowResize();
		}));

	m_EventDispatcher = eventDispatcher;
	m_CallbackId = m_EventDispatcher->Register(render::E_RenderEvent::RenderOutlines, render::T_RenderEventCallback(
		[this](render::T_RenderEventFlags const flags, render::RenderEventData const* const evnt) -> void
		{
			UNUSED(flags);

			if (evnt->renderer->GetType() == rttr::type::get<render::ShadedSceneRenderer>())
			{
				render::ShadedSceneRenderer const* const renderer = static_cast<render::ShadedSceneRenderer const*>(evnt->renderer);
				render::I_SceneExtension const* const ext = renderer->GetScene()->GetExtension(core::HashString("OutlineExtension"));
				if (ext == nullptr)
				{
					LOG("render scene does not have an outline extension");
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

	if (m_VPCallbackId != render::T_ViewportEventDispatcher::INVALID_ID)
	{
		Viewport::GetCurrentViewport()->GetEventDispatcher().Unregister(m_VPCallbackId);
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
	I_GraphicsApiContext* const api = Viewport::GetCurrentApiContext();
	ivec2 const dim = Viewport::GetCurrentViewport()->GetDimensions();

	TextureParameters params(false);
	params.minFilter = E_TextureFilterMode::Linear;
	params.magFilter = E_TextureFilterMode::Linear;
	params.wrapS = E_TextureWrapMode::ClampToBorder;
	params.wrapT = E_TextureWrapMode::ClampToBorder;
	params.borderColor = vec4(vec3(0.f), 1.f);

	//Generate texture and fbo and rbo as initial postprocessing target
	api->GenFramebuffers(1, &m_DrawTarget);
	api->BindFramebuffer(m_DrawTarget);
	m_DrawTex = new TextureData(dim, E_ColorFormat::RGB16f, E_ColorFormat::RGB, E_DataType::Float);
	m_DrawTex->Build();
	m_DrawTex->SetParameters(params);
	api->LinkTextureToFbo2D(0, m_DrawTex->GetLocation(), 0);

	api->GenRenderBuffers(1, &m_DrawDepth);
	api->BindRenderbuffer(m_DrawDepth);
	api->SetRenderbufferStorage(E_RenderBufferFormat::Depth24, dim);
	api->LinkRenderbufferToFbo(E_RenderBufferFormat::Depth24, m_DrawDepth);

	api->BindFramebuffer(0);
}

//---------------------------------
// OutlineRenderer::DestroyRenderTarget
//
void OutlineRenderer::DestroyRenderTarget()
{
	I_GraphicsApiContext* const api = Viewport::GetCurrentApiContext();

	api->DeleteRenderBuffers(1, &m_DrawDepth);
	SafeDelete(m_DrawTex);
	api->DeleteFramebuffers(1, &m_DrawTarget);
}

//---------------------------------
// OutlineRenderer::Draw
//
void OutlineRenderer::Draw(T_FbLoc const targetFb, 
	OutlineExtension const& outlines, 
	core::slot_map<mat4> const& nodes, 
	Camera const& cam, 
	Gbuffer const& gbuffer)
{
	if (outlines.GetOutlineLists().empty())
	{
		return;
	}

	I_GraphicsApiContext* const api = Viewport::GetCurrentApiContext();
	ivec2 const dim = Viewport::GetCurrentViewport()->GetDimensions();

	api->SetViewport(ivec2(0), dim);

	// draw the shapes as colors to the intermediate rendertarget
	//------------------------------------------------------------
	api->BindFramebuffer(m_DrawTarget);

	api->SetClearColor(vec4(vec3(0.f), 1.f));
	api->Clear(E_ClearFlag::Color | E_ClearFlag::Depth);

	render::Material const* const mat = RenderingSystems::Instance()->GetColorMaterial();
	ShaderData const* const shader = mat->GetShader();

	api->SetShader(shader);
	shader->Upload("uViewSize"_hash, math::vecCast<float>(dim));

	shader->Upload("uOcclusionFactor"_hash, 0.15f);

	api->SetDepthEnabled(true); 

	for (OutlineExtension::OutlineList const& list : outlines.GetOutlineLists())
	{
		shader->Upload("uColor"_hash, list.color);

		for (render::MaterialCollection::Mesh const& mesh : list.meshes)
		{
			api->BindVertexArray(mesh.m_VAO);
			for (render::T_NodeId const node : mesh.m_Instances)
			{
				// #todo: collect a list of transforms and draw this instanced
				mat4 const& transform = nodes[node];
				math::Sphere instSphere = math::Sphere((transform * vec4(mesh.m_BoundingVolume.pos, 1.f)).xyz,
					math::length(math::decomposeScale(transform)) * mesh.m_BoundingVolume.radius);

				if (cam.GetFrustum().ContainsSphere(instSphere) != VolumeCheck::OUTSIDE)
				{
					shader->Upload("model"_hash, transform);
					api->DrawElements(E_DrawMode::Triangles, mesh.m_IndexCount, mesh.m_IndexDataType, 0);
				}
			}
		}
	}

	// apply a sobel shader to the colored shapes and render it to the target framebuffer
	//------------------------------------------------------------------------------------

	api->BindFramebuffer(targetFb);

	api->SetDepthEnabled(false);
	api->SetBlendEnabled(true);
	api->SetBlendEquation(E_BlendEquation::Add);
	api->SetBlendFunction(E_BlendFactor::One, E_BlendFactor::One);

	api->SetShader(m_SobelShader.get());

	m_SobelShader->Upload("inColorTex"_hash, static_cast<TextureData const*>(m_DrawTex));

	RenderingSystems::Instance()->GetPrimitiveRenderer().Draw<primitives::Quad>();

	api->SetBlendEnabled(false);
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
