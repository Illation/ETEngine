#include "stdafx.h"
#include "SceneRendererGUI.h"

#include <EtRendering/SceneStructure/RenderScene.h>
#include <EtRendering/SceneRendering/ShadedSceneRenderer.h>


namespace et {
namespace gui {


//====================
// Scene Renderer GUI
//====================


//---------------------------------
// SceneRendererGUI::d-tor
//
SceneRendererGUI::~SceneRendererGUI()
{
	if (m_IsInitialized)
	{
		Deinit();
	}
}

//---------------------------------
// SceneRendererGUI::Init
//
void SceneRendererGUI::Init(Ptr<render::T_RenderEventDispatcher> const eventDispatcher)
{
	// basic rendering systems
	//-------------------------
	m_TextRenderer.Initialize();
	m_SpriteRenderer.Initialize();
	m_GuiRenderer.Init();

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
				render::I_SceneExtension* const ext = renderer->GetScene()->GetExtension(GuiExtension::s_ExtensionId);
				if (ext == nullptr)
				{
					LOG("render scene does not have a GUI extension");
					return;
				}

				GuiExtension* const guiExt = static_cast<GuiExtension*>(ext);
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
// SceneRendererGUI::Deinit
//
void SceneRendererGUI::Deinit()
{
	m_GuiRenderer.Deinit();
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
// SceneRendererGUI::DrawInWorld
//
// Draw UI from the extension that sits within the world
//
void SceneRendererGUI::DrawInWorld(render::T_FbLoc const targetFb, GuiExtension& guiExt, core::slot_map<mat4> const& nodes)
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

	// RmlUi
	//--------
	api->DebugPushGroup("RmlUi");

	for (ContextContainer::WorldContext& worldContext : guiExt.GetContextContainer().GetWorldContexts())
	{
		if (worldContext.m_Context.IsActive() && worldContext.m_Context.IsDocumentLoaded())
		{
			ivec2 const contextDim = worldContext.m_Context.GetDimensions();
			worldContext.m_RenderTarget.UpdateForDimensions(contextDim);

			mat4 const transform = math::scale(vec3(math::vecCast<float>(contextDim), 1.f)) * nodes[worldContext.m_NodeId];

			m_GuiRenderer.RenderWorldContext(targetFb, worldContext.m_RenderTarget, worldContext.m_Context, transform, worldContext.m_IsDepthEnabled);
		}
	}

	api->DebugPopGroup(); // RmlUi
}

//---------------------------------
// SceneRendererGUI::DrawOverlay
//
// Draw UI from the extension that goes on top of everything else
//
void SceneRendererGUI::DrawOverlay(render::T_FbLoc const targetFb, GuiExtension& guiExt)
{
	render::I_GraphicsContextApi* const api = render::ContextHolder::GetRenderContext();
	api->BindFramebuffer(targetFb);

	m_SpriteRenderer.Draw();
	m_TextRenderer.Draw();

	// RmlUi
	//--------
	api->DebugPushGroup("RmlUi");

	render::Viewport const* const viewport = render::Viewport::GetCurrentViewport();
	ContextRenderTarget* renderTarget;
	ContextContainer::T_Contexts& contexts = guiExt.GetContextContainer().GetContexts(viewport, renderTarget);
	renderTarget->UpdateForDimensions(viewport->GetDimensions());
	m_GuiRenderer.RenderContexts(targetFb, *renderTarget, contexts.data(), contexts.size());

	api->DebugPopGroup(); // RmlUi
}


} // namespace gui
} // namespace et

