#include "stdafx.h"
#include "GuiRenderer.h"

#include <EtRendering/SceneStructure/RenderScene.h>
#include <EtRendering/SceneRendering/ShadedSceneRenderer.h>

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

	// render events
	//---------------
	m_EventDispatcher = eventDispatcher;

	// In World
	m_WorldCallbackId = m_EventDispatcher->Register(render::E_RenderEvent::RenderWorldGUI, render::T_RenderEventCallback(
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
	m_OverlayCallbackId = m_EventDispatcher->Register(render::E_RenderEvent::RenderOverlay, render::T_RenderEventCallback(
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
void GuiRenderer::DrawOverlay(render::T_FbLoc const targetFb, GuiExtension const& guiExt)
{
	render::I_GraphicsContextApi* const api = render::ContextHolder::GetRenderContext();
	api->BindFramebuffer(targetFb);

	m_SpriteRenderer.Draw();
	m_TextRenderer.Draw();

	ContextContainer::T_Contexts const& contexts = guiExt.GetContextContainer().GetContexts(render::Viewport::GetCurrentViewport());
	for (Context const& context : contexts)
	{
		// render the context
		UNUSED(context);
	}
}


} // namespace gui
} // namespace et

