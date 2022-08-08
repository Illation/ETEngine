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
	m_GuiRenderer.Init();

	// render events
	//---------------
	m_EventDispatcher = eventDispatcher;

	// In World
	m_WorldCallbackId = m_EventDispatcher->Register(render::E_RenderEvent::RE_RenderWorldGUI, render::T_RenderEventCallback(
		[this](render::T_RenderEventFlags const flags, render::RenderEventData const* const evnt) -> void
		{
			ET_UNUSED(flags);

			if (evnt->renderer->GetType() == rttr::type::get<render::ShadedSceneRenderer>())
			{
				render::ShadedSceneRenderer const* const renderer = static_cast<render::ShadedSceneRenderer const*>(evnt->renderer);
				render::I_SceneExtension* const ext = renderer->GetScene()->GetExtension(GuiExtension::s_ExtensionId);
				if (ext == nullptr)
				{
					ET_TRACE_V(ET_CTX_GUI, "render scene does not have a GUI extension");
					return;
				}

				GuiExtension* const guiExt = static_cast<GuiExtension*>(ext);
				DrawInWorld(evnt->targetFb, renderer->Get3DPolyMode(), *guiExt, renderer->GetScene()->GetNodes());
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
			ET_UNUSED(flags);

			if (evnt->renderer->GetType() == rttr::type::get<render::ShadedSceneRenderer>())
			{
				render::ShadedSceneRenderer const* const renderer = static_cast<render::ShadedSceneRenderer const*>(evnt->renderer);
				render::I_SceneExtension* const ext = renderer->GetScene()->GetExtension(GuiExtension::s_ExtensionId);
				if (ext == nullptr)
				{
					ET_TRACE_V(ET_CTX_GUI, "render scene does not have a GUI extension");
					return;
				}

				GuiExtension* const guiExt = static_cast<GuiExtension*>(ext);
				DrawOverlay(evnt->targetFb, renderer->Get3DPolyMode(), *guiExt);
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
void SceneRendererGUI::DrawInWorld(rhi::T_FbLoc const targetFb, 
	rhi::E_PolygonMode const polyMode,
	GuiExtension& guiExt, 
	core::slot_map<mat4> const& nodes)
{
	rhi::I_GraphicsContextApi* const api = rhi::ContextHolder::GetRenderContext();
	api->BindFramebuffer(targetFb);

	// RmlUi 3D contexts
	//-------------------
	for (GuiExtension::WorldContext& worldContext : guiExt.GetWorldContexts())
	{
		if (worldContext.m_Context.HasActiveDocuments())
		{
			ivec2 const contextDim = worldContext.m_Context.GetDimensions();
			worldContext.m_RenderTarget.UpdateForDimensions(contextDim);

			// the quad drawn is 2x2 so we need to halve the size
			mat4 const transform = math::scale(vec3(math::vecCast<float>(contextDim) * 0.5f, 1.f)) * nodes[worldContext.m_NodeId];

			m_GuiRenderer.RenderWorldContext(targetFb, 
				worldContext.m_RenderTarget, 
				worldContext.m_Context, 
				transform, 
				worldContext.m_Color, 
				worldContext.m_IsDepthEnabled,
				polyMode);
		}
	}
}

//---------------------------------
// SceneRendererGUI::DrawOverlay
//
// Draw UI from the extension that goes on top of everything else
//
void SceneRendererGUI::DrawOverlay(rhi::T_FbLoc const targetFb, rhi::E_PolygonMode const polyMode, GuiExtension& guiExt)
{
	rhi::I_GraphicsContextApi* const api = rhi::ContextHolder::GetRenderContext();
	api->BindFramebuffer(targetFb);

	// RmlUi overlays
	//----------------
	rhi::Viewport const* const viewport = rhi::Viewport::GetCurrentViewport();
	ContextRenderTarget* renderTarget;
	Context* const context = guiExt.GetContext(viewport, renderTarget);
	if ((context != nullptr) && context->HasActiveDocuments())
	{
		renderTarget->UpdateForDimensions(viewport->GetDimensions());
		m_GuiRenderer.RenderContext(targetFb, *renderTarget, *context, polyMode);
	}
}


} // namespace gui
} // namespace et

