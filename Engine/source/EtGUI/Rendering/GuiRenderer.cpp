#include "stdafx.h"
#include "GuiRenderer.h"

#include <EtCore/Content/ResourceManager.h>

#include <EtRendering/SceneStructure/RenderScene.h>
#include <EtRendering/SceneRendering/ShadedSceneRenderer.h>
#include <EtRendering/GraphicsTypes/Shader.h>
#include <EtRendering/GlobalRenderingSystems/GlobalRenderingSystems.h>

#include <EtGUI/Context/RmlGlobal.h>


namespace et {
namespace gui {


//===============
// GUI Renderer
//===============


//---------------------------------
// GuiRenderer::d-tor
//
GuiRenderer::~GuiRenderer()
{
	if (m_IsInitialized)
	{
		Deinit();
	}
}

//---------------------------------
// GuiRenderer::Init
//
void GuiRenderer::Init(Ptr<render::T_RenderEventDispatcher> const eventDispatcher)
{
	// basic rendering systems
	//-------------------------
	m_TextRenderer.Initialize();
	m_SpriteRenderer.Initialize();

	// Rml UI
	//--------
	m_RmlGlobal = RmlGlobal::GetInstance(); // might initialize RML if this is the first GUI renderer
	m_RmlShader = core::ResourceManager::Instance()->GetAssetData<render::ShaderData>(core::HashString("Shaders/PostRmlUi.glsl"));
	m_RmlSdfShader = core::ResourceManager::Instance()->GetAssetData<render::ShaderData>(core::HashString("Shaders/PostRmlUiSdf.glsl"));
	m_RmlBlitShader = core::ResourceManager::Instance()->GetAssetData<render::ShaderData>(core::HashString("Shaders/PostRmlUiBlit.glsl"));

	// render events
	//---------------
	m_EventDispatcher = eventDispatcher;

	// In World
	m_WorldCallbackId = m_EventDispatcher->Register(render::E_RenderEvent::RE_RenderWorldGUI, render::T_RenderEventCallback(
		[this](render::T_RenderEventFlags const flags, render::RenderEventData const* const evnt) -> void
		{
			UNUSED(flags);

			if (evnt->renderer->GetType() == rttr::type::get<render::ShadedSceneRenderer>())
			{
				render::ShadedSceneRenderer const* const renderer = static_cast<render::ShadedSceneRenderer const*>(evnt->renderer);
				render::I_SceneExtension const* const ext = renderer->GetScene()->GetExtension(GuiExtension::s_ExtensionId);
				if (ext == nullptr)
				{
					LOG("render scene does not have a GUI extension");
					return;
				}

				GuiExtension const* const guiExt = static_cast<GuiExtension const*>(ext);
				DrawInWorld(evnt->targetFb, *guiExt, renderer->GetScene()->GetNodes());
			}
			else
			{
				ET_ASSERT(true, "Cannot retrieve GUI info from unhandled renderer!");
			}
		}));

	// Overlay
	m_OverlayCallbackId = m_EventDispatcher->Register(render::E_RenderEvent::RE_RenderOverlay, render::T_RenderEventCallback(
		[this](render::T_RenderEventFlags const flags, render::RenderEventData const* const evnt) -> void
		{
			UNUSED(flags);

			if (evnt->renderer->GetType() == rttr::type::get<render::ShadedSceneRenderer>())
			{
				render::ShadedSceneRenderer const* const renderer = static_cast<render::ShadedSceneRenderer const*>(evnt->renderer);
				render::I_SceneExtension* const ext = renderer->GetScene()->GetExtension(GuiExtension::s_ExtensionId);
				if (ext == nullptr)
				{
					LOG("render scene does not have a GUI extension");
					return;
				}

				GuiExtension* const guiExt = static_cast<GuiExtension*>(ext);
				DrawOverlay(evnt->targetFb, *guiExt);
			}
			else
			{
				ET_ASSERT(true, "Cannot retrieve GUI info from unhandled renderer!");
			}
		}));

	m_IsInitialized = true;
}

//---------------------------------
// GuiRenderer::Deinit
//
void GuiRenderer::Deinit()
{
	DeleteFramebuffer();

	m_RmlGlobal = nullptr;

	m_SpriteRenderer.Deinit();
	m_TextRenderer.Deinit();

	if (m_EventDispatcher != nullptr)
	{
		m_EventDispatcher->Unregister(m_WorldCallbackId);
		m_EventDispatcher->Unregister(m_OverlayCallbackId);
		m_EventDispatcher = nullptr;
	}

	m_IsInitialized = false;
}

//---------------------------------
// GuiRenderer::DrawInWorld
//
// Draw UI from the extension that sits within the world
//
void GuiRenderer::DrawInWorld(render::T_FbLoc const targetFb, GuiExtension const& guiExt, core::slot_map<mat4> const& nodes)
{
	render::I_GraphicsContextApi* const api = render::ContextHolder::GetRenderContext();
	api->BindFramebuffer(targetFb);

	SpriteRenderer::E_ScalingMode const scalingMode = SpriteRenderer::E_ScalingMode::Texture;
	for (GuiExtension::Sprite const& sprite : guiExt.GetSprites())
	{
		mat4 const& transform = nodes[sprite.node];

		vec3 pos, scale;
		quat rot;
		math::decomposeTRS(transform, pos, rot, scale);

		m_SpriteRenderer.Draw(sprite.texture, pos.xy, sprite.color, sprite.pivot, scale.xy, rot.Roll(), pos.z, scalingMode);
	}
}

//---------------------------------
// GuiRenderer::DrawOverlay
//
// Draw UI from the extension that goes on top of everything else
//
void GuiRenderer::DrawOverlay(render::T_FbLoc const targetFb, GuiExtension& guiExt)
{
	render::I_GraphicsContextApi* const api = render::ContextHolder::GetRenderContext();
	api->BindFramebuffer(targetFb);

	m_SpriteRenderer.Draw();
	m_TextRenderer.Draw();

	// RmlUi
	//--------
	api->DebugPushGroup("RmlUi");

	RmlGlobal::GetInstance()->SetGraphicsContext(ToPtr(api));

	render::Viewport const* const viewport = render::Viewport::GetCurrentViewport();

	// Set target framebuffer
	ivec2 const iViewDim = viewport->GetDimensions();
	if ((m_RmlTex == nullptr) || !math::nearEqualsV(m_RmlTex->GetResolution(), iViewDim))
	{
		GenerateFramebuffer(iViewDim);
	}

	api->BindFramebuffer(m_RmlFb);

	api->SetClearColor(vec4(0.f));
	api->Clear(render::E_ClearFlag::CF_Color);

	// view projection matrix
	vec2 const viewDim = math::vecCast<float>(iViewDim);
	mat4 const viewProjection = math::orthographic(0.f, viewDim.x, viewDim.y, 0.f, -10000.f, 10000.f) * math::scale(vec3(1.f, -1.f, 1.f)); // v flip
	RmlGlobal::GetInstance()->SetRIView(iViewDim, viewProjection);

	// set shaders
	api->SetShader(m_RmlSdfShader.get());
	m_RmlSdfShader->Upload("uViewProjection"_hash, viewProjection);

	api->SetShader(m_RmlShader.get());
	m_RmlShader->Upload("uViewProjection"_hash, viewProjection);

	RmlGlobal::GetInstance()->SetRIShader(m_RmlShader, m_RmlSdfShader);

	// pipeline state
	api->SetBlendEnabled(true);
	api->SetBlendEquation(render::E_BlendEquation::Add);
	api->SetBlendFunction(render::E_BlendFactor::SourceAlpha, render::E_BlendFactor::OneMinusSourceAlpha);
	api->SetCullEnabled(false);
	api->SetDepthEnabled(false);

	// render each context
	ContextContainer::T_Contexts& contexts = guiExt.GetContextContainer().GetContexts(viewport);
	for (Context& context : contexts)
	{
		if (context.IsActive() && context.IsDocumentLoaded())
		{
			context.Render();
		}
	}

	// blit results to target framebuffer - this can also be used in the future to transform the UI into viewspace
	api->BindFramebuffer(targetFb);

	api->SetShader(m_RmlBlitShader.get());
	m_RmlBlitShader->Upload("uTexture"_hash, static_cast<render::TextureData const*>(m_RmlTex.Get()));
	render::RenderingSystems::Instance()->GetPrimitiveRenderer().Draw<render::primitives::Quad>();

	// reset pipeline state
	api->SetBlendEnabled(false);

	api->DebugPopGroup(); // RmlUi
}

//----------------------------------
// GuiRenderer::GenerateFramebuffer
//
void GuiRenderer::GenerateFramebuffer(ivec2 const dim)
{
	DeleteFramebuffer();

	render::I_GraphicsContextApi* const api = render::ContextHolder::GetRenderContext();

	api->GenFramebuffers(1, &m_RmlFb);
	api->BindFramebuffer(m_RmlFb);

	// target texture
	m_RmlTex = Create<render::TextureData>(render::E_ColorFormat::RGBA16f, dim);
	m_RmlTex->AllocateStorage();
	m_RmlTex->SetParameters(render::TextureParameters(false));

	//Render Buffer for depth and stencil
	api->GenRenderBuffers(1, &m_RmlRbo);
	api->BindRenderbuffer(m_RmlRbo);
	api->SetRenderbufferStorage(render::E_RenderBufferFormat::Depth24_Stencil8, dim);

	// link it all together
	api->LinkRenderbufferToFbo(render::E_RenderBufferFormat::Depth24_Stencil8, m_RmlRbo);
	api->LinkTextureToFbo2D(0, m_RmlTex->GetLocation(), 0);

	api->BindFramebuffer(0u);
}

//--------------------------------
// GuiRenderer::DeleteFramebuffer
//
void GuiRenderer::DeleteFramebuffer()
{
	if (m_RmlTex != nullptr)
	{
		render::I_GraphicsContextApi* const api = render::ContextHolder::GetRenderContext();

		api->DeleteRenderBuffers(1, &m_RmlRbo);
		m_RmlTex = nullptr;
		api->DeleteFramebuffers(1, &m_RmlFb);
	}
}


} // namespace gui
} // namespace et

