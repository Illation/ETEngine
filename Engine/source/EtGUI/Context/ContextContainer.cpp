#include "stdafx.h"
#include "ContextContainer.h"

#include "RmlUtil.h"
#include "RmlGlobal.h"

#include <EtCore/Input/RawInputProvider.h>

#include <EtRendering/GraphicsContext/Viewport.h>

#ifdef ET_DEBUG
#	include <RmlUi/Debugger.h>
#endif


namespace et {
namespace gui {


//===================================
// Context Container :: Per Viewport
//===================================


//--------------------------------
// PerViewport::ProcessKeyPressed
//
bool ContextContainer::PerViewport::ProcessKeyPressed(E_KbdKey const key, core::T_KeyModifierFlags const modifiers)
{
	if (!m_ContextContainer->IsInputEnabled())
	{
		return false;
	}

	Rml::Input::KeyIdentifier const rmlKey = RmlUtil::GetRmlKeyId(key);
	if (rmlKey != Rml::Input::KeyIdentifier::KI_UNKNOWN)
	{
		int32 const mods = RmlUtil::GetRmlModifierFlags(modifiers);
		if (m_Context.HasActiveDocuments())
		{
			if (m_Context.ProcessKeyPressed(rmlKey, mods))
			{
				return true;
			}
		}

		for (core::T_SlotId const worldContextId : m_EventWorldContexts)
		{
			WorldContext& worldContext = m_ContextContainer->GetWorldContexts()[worldContextId];
			if (worldContext.m_Context.HasActiveDocuments())
			{
				if (m_ContextContainer->GetRenderScene()->GetCameras()[worldContext.m_EventCameraId].GetViewport() == m_Viewport)
				{
					if (worldContext.m_Context.ProcessKeyPressed(rmlKey, mods))
					{
						return true;
					}
				}
			}
		}
	}

	return false;
}

//---------------------------------
// PerViewport::ProcessKeyReleased
//
bool ContextContainer::PerViewport::ProcessKeyReleased(E_KbdKey const key, core::T_KeyModifierFlags const modifiers)
{
	if (!m_ContextContainer->IsInputEnabled())
	{
		return false;
	}

	Rml::Input::KeyIdentifier const rmlKey = RmlUtil::GetRmlKeyId(key);
	if (rmlKey != Rml::Input::KeyIdentifier::KI_UNKNOWN)
	{
		int32 const mods = RmlUtil::GetRmlModifierFlags(modifiers);
		if (m_Context.HasActiveDocuments())
		{
			if (m_Context.ProcessKeyReleased(rmlKey, mods))
			{
				return true;
			}
		}

		for (core::T_SlotId const worldContextId : m_EventWorldContexts)
		{
			WorldContext& worldContext = m_ContextContainer->GetWorldContexts()[worldContextId];
			if (worldContext.m_Context.HasActiveDocuments())
			{
				if (m_ContextContainer->GetRenderScene()->GetCameras()[worldContext.m_EventCameraId].GetViewport() == m_Viewport)
				{
					if (worldContext.m_Context.ProcessKeyReleased(rmlKey, mods))
					{
						return true;
					}
				}
			}
		}
	}

	return false;
}

//----------------------------------
// PerViewport::ProcessMousePressed
//
bool ContextContainer::PerViewport::ProcessMousePressed(E_MouseButton const button, core::T_KeyModifierFlags const modifiers)
{
	if (!m_ContextContainer->IsInputEnabled())
	{
		return false;
	}

	int32 const rmlButton = RmlUtil::GetRmlButtonIndex(button);
	if (rmlButton != -1)
	{
		int32 const mods = RmlUtil::GetRmlModifierFlags(modifiers);
		if (m_Context.HasActiveDocuments())
		{
			if (m_Context.ProcessMousePressed(rmlButton, mods))
			{
				return true;
			}
		}

		for (core::T_SlotId const worldContextId : m_EventWorldContexts)
		{
			WorldContext& worldContext = m_ContextContainer->GetWorldContexts()[worldContextId];
			if (worldContext.m_Context.HasActiveDocuments())
			{
				if (m_ContextContainer->GetRenderScene()->GetCameras()[worldContext.m_EventCameraId].GetViewport() == m_Viewport)
				{
					if (worldContext.m_Context.ProcessMousePressed(rmlButton, mods))
					{
						return true;
					}
				}
			}
		}
	}

	return false;
}

//-----------------------------------
// PerViewport::ProcessMouseReleased
//
bool ContextContainer::PerViewport::ProcessMouseReleased(E_MouseButton const button, core::T_KeyModifierFlags const modifiers)
{
	if (!m_ContextContainer->IsInputEnabled())
	{
		return false;
	}

	int32 const rmlButton = RmlUtil::GetRmlButtonIndex(button);
	if (rmlButton != -1)
	{
		int32 const mods = RmlUtil::GetRmlModifierFlags(modifiers);
		if (m_Context.HasActiveDocuments())
		{
			if (m_Context.ProcessMouseReleased(rmlButton, mods))
			{
				return true;
			}
		}

		for (core::T_SlotId const worldContextId : m_EventWorldContexts)
		{
			WorldContext& worldContext = m_ContextContainer->GetWorldContexts()[worldContextId];
			if (worldContext.m_Context.HasActiveDocuments())
			{
				if (m_ContextContainer->GetRenderScene()->GetCameras()[worldContext.m_EventCameraId].GetViewport() == m_Viewport)
				{
					if (worldContext.m_Context.ProcessMouseReleased(rmlButton, mods))
					{
						return true;
					}
				}
			}
		}
	}

	return false;
}

//-------------------------------
// PerViewport::ProcessMouseMove
//
bool ContextContainer::PerViewport::ProcessMouseMove(ivec2 const& mousePos, core::T_KeyModifierFlags const modifiers)
{
	if (!m_ContextContainer->IsInputEnabled())
	{
		return false;
	}

	int32 const mods = RmlUtil::GetRmlModifierFlags(modifiers);
	if (m_Context.HasActiveDocuments())
	{
		if (m_Context.ProcessMouseMove(mousePos, mods))
		{
			return true;
		}
	}

	render::Scene const* const renderScene = m_ContextContainer->GetRenderScene();
	for (core::T_SlotId const worldContextId : m_EventWorldContexts)
	{
		WorldContext& worldContext = m_ContextContainer->GetWorldContexts()[worldContextId];
		if (worldContext.m_Context.HasActiveDocuments())
		{
			render::Camera const& camera = renderScene->GetCameras()[worldContext.m_EventCameraId];
			if (camera.GetViewport() == m_Viewport)
			{
				vec2 const contextDim = math::vecCast<float>(worldContext.m_Context.GetDimensions());

				// construct a rectangle on a plane from the contexts transform and dimensions
				mat4 const transform = math::scale(vec3(contextDim, 1.f)) * renderScene->GetNodes()[worldContext.m_NodeId];
				mat4 const orientation = math::DiscardW(transform);

				vec3 const xTF = (orientation * vec4(vec3::RIGHT, 1.f)).xyz; // horizontal edge of the context rectangle
				vec3 const yTF = (orientation * vec4(vec3::DOWN, 1.f)).xyz; // vertical edge of the context rectangle
				// plane with determinant (p0) in top left corner
				math::Plane const plane(math::normalize(math::cross(xTF, yTF)), math::decomposePosition(transform) + (xTF * -0.5f) + (yTF * -0.5f));

				// convert mouse pos into a ray from the camera
				vec3 const rayDir = math::normalize(
					camera.ProjectIntoWorldSpace(math::vecCast<float>(mousePos) / math::vecCast<float>(m_Viewport->GetDimensions()), 0.f) 
					- camera.GetPosition());

				// intersect the ray with the plane
				vec3 intersection;
				if (math::GetIntersection(plane, camera.GetPosition(), rayDir, intersection))
				{
					// now project the difference vector of the intersection and rectangle origin onto the rectangle edges
					vec3 const p0p = intersection - plane.d;
					float const qx = math::vecProjectionFactor(p0p, xTF);
					float const qy = math::vecProjectionFactor(p0p, yTF);

					ivec2 const contextMousePos = math::vecCast<int32>(vec2(qx, qy) * contextDim);

					if (worldContext.m_Context.ProcessMouseMove(contextMousePos, mods))
					{
						return true;
					}
				}
			}
		}
	}

	return false;
}

//-------------------------------------
// PerViewport::ProcessMouseWheelDelta
//
bool ContextContainer::PerViewport::ProcessMouseWheelDelta(ivec2 const& mouseWheel, core::T_KeyModifierFlags const modifiers)
{
	if (!m_ContextContainer->IsInputEnabled())
	{
		return false;
	}

	ivec2 const delta(mouseWheel.x, -mouseWheel.y);
	int32 const mods = RmlUtil::GetRmlModifierFlags(modifiers);
	if (m_Context.HasActiveDocuments())
	{
		if (m_Context.ProcessMouseWheelDelta(delta, mods))
		{
			return true;
		}
	}

	for (core::T_SlotId const worldContextId : m_EventWorldContexts)
	{
		WorldContext& worldContext = m_ContextContainer->GetWorldContexts()[worldContextId];
		if (worldContext.m_Context.HasActiveDocuments())
		{
			if (m_ContextContainer->GetRenderScene()->GetCameras()[worldContext.m_EventCameraId].GetViewport() == m_Viewport)
			{
				if (worldContext.m_Context.ProcessMouseWheelDelta(delta, mods))
				{
					return true;
				}
			}
		}
	}

	return false;
}

//---------------------------------
// PerViewport::ProcessTextInput
//
bool ContextContainer::PerViewport::ProcessTextInput(core::E_Character const character)
{
	if (!m_ContextContainer->IsInputEnabled())
	{
		return false;
	}

	if (m_Context.HasActiveDocuments())
	{
		if (m_Context.ProcessTextInput(static_cast<Rml::Character>(character)))
		{
			return true;
		}
	}

	for (core::T_SlotId const worldContextId : m_EventWorldContexts)
	{
		WorldContext& worldContext = m_ContextContainer->GetWorldContexts()[worldContextId];
		if (worldContext.m_Context.HasActiveDocuments())
		{
			if (worldContext.m_Context.ProcessTextInput(static_cast<Rml::Character>(character)))
			{
				return true;
			}
		}
	}

	return false;
}


//===================
// Context Container
//===================


//---------------------------------
// ContextContainer::OnTick
//
void ContextContainer::OnTick()
{
	// screen contexts
	for (auto& el : m_ViewportContexts)
	{
		if (el.second.m_Context.HasActiveDocuments())
		{
			el.second.m_Context.Update();
		}
	}

	// world contexts
	for (WorldContext& worldContext : m_WorldContexts)
	{
		Context& context = worldContext.m_Context;
		if (context.HasActiveDocuments())
		{
			context.Update();
		}
	}
}

//---------------------------------
// ContextContainer::CreateContext
//
// Create a screenspace GUI context attached to a specific viewport
//  - may create a new context list and bind for viewport resize events
//
T_ContextId ContextContainer::CreateContext(Ptr<render::Viewport> const viewport)
{
	auto const ret = m_Contexts.insert(ContextData());

	PerViewport& perVp = FindOrCreatePerViewport(viewport);
	auto const context = perVp.m_Documents.insert(core::HashString());

	ContextData& ctxData = *ret.first;
	ctxData.m_Viewport = viewport;
	ctxData.m_Context = context.second;
	ctxData.m_IsViewportContext = true;

	perVp.m_Context.Init(std::to_string(ret.second), viewport->GetDimensions());
	return ret.second;
}

//---------------------------------
// ContextContainer::CreateContext
//
// Create a worldspace context
//
T_ContextId ContextContainer::CreateContext(core::T_SlotId const nodeId, ivec2 const dimensions)
{
	auto const ret = m_Contexts.insert(ContextData());

	auto const worldContext = m_WorldContexts.insert(WorldContext());

	ContextData& ctxData = *ret.first;
	ctxData.m_Viewport = nullptr;
	ctxData.m_Context = worldContext.second;
	ctxData.m_IsViewportContext = false;

	worldContext.first->m_Context.Init(std::to_string(ret.second), dimensions);
	worldContext.first->m_NodeId = nodeId;

	return ret.second;
}

//----------------------------------
// ContextContainer::DestroyContext
//
// will delete the last PerViewport data and event bindings
//
void ContextContainer::DestroyContext(T_ContextId const id)
{
	ContextData& ctxData = m_Contexts[id];

	if (ctxData.m_IsViewportContext)
	{
		T_ViewportContexts::iterator const found = m_ViewportContexts.find(ctxData.m_Viewport);
		ET_ASSERT(found != m_ViewportContexts.cend());
		if ((found->second.m_Documents.size() == 1u) && (found->second.m_EventWorldContexts.empty()))
		{
			ErasePerViewport(ctxData.m_Viewport.Get(), found);
		}
		else
		{
			found->second.m_Documents.erase(ctxData.m_Context);
		}
	}
	else
	{
		SetEventCamera(id, core::INVALID_SLOT_ID);
		m_WorldContexts.erase(ctxData.m_Context);
	}

	m_Contexts.erase(id);
}

//------------------------------------
// ContextContainer::SetContextActive
//
void ContextContainer::SetContextActive(T_ContextId const id, bool const isActive)
{
	ContextData& ctxData = m_Contexts[id];

	if (ctxData.m_IsViewportContext)
	{
		T_ViewportContexts::iterator const found = m_ViewportContexts.find(ctxData.m_Viewport);
		ET_ASSERT(found != m_ViewportContexts.cend());
		found->second.m_Context.SetDocumentActive(found->second.m_Documents[ctxData.m_Context], isActive);
	}
	else
	{
		Context& context = m_WorldContexts[ctxData.m_Context].m_Context;
		context.SetDocumentActive(context.GetDocumentId(0u), isActive);
	}
}

//----------------------------------
// ContextContainer::SetEventCamera
//
// for 3D contexts set the camera we receive events from
// 
void ContextContainer::SetEventCamera(T_ContextId const id, core::T_SlotId const cameraId)
{
	ContextData& ctxData = m_Contexts[id];
	ET_ASSERT(!ctxData.m_IsViewportContext, "Context event camera is only applicable on World contexts");

	WorldContext& worldContext = m_WorldContexts[ctxData.m_Context];
	if (worldContext.m_EventCameraId != cameraId)
	{
		if (worldContext.m_EventCameraId != core::INVALID_SLOT_ID)
		{
			T_ViewportContexts::iterator found = m_ViewportContexts.find(ctxData.m_Viewport);
			ET_ASSERT(found != m_ViewportContexts.cend());
			if ((found->second.m_EventWorldContexts.size() == 1u) && (found->second.m_Documents.empty()))
			{
				ErasePerViewport(ctxData.m_Viewport.Get(), found);
			}
			else
			{
				found->second.m_EventWorldContexts.erase(ctxData.m_Context);
			}

			ctxData.m_Viewport = nullptr;
		}

		worldContext.m_EventCameraId = cameraId;

		if (worldContext.m_EventCameraId != core::INVALID_SLOT_ID)
		{
			render::Camera const& cam = m_RenderScene->GetCameras()[worldContext.m_EventCameraId];
			ET_ASSERT(cam.GetViewport() != nullptr);

			PerViewport& perVp = FindOrCreatePerViewport(cam.GetViewport());
			perVp.m_EventWorldContexts.insert(ctxData.m_Context);

			ctxData.m_Viewport = cam.GetViewport();
		}
	}
}

//-----------------------------------
// ContextContainer::SetContextColor
//
// for 3D contexts the colour all UI will be multiplied with
//
void ContextContainer::SetContextColor(T_ContextId const id, vec4 const& color)
{
	ContextData& ctxData = m_Contexts[id];
	ET_ASSERT(!ctxData.m_IsViewportContext, "Context color is only applicable on World contexts");

	m_WorldContexts[ctxData.m_Context].m_Color = color;
}

//---------------------------------------
// ContextContainer::SetDepthTestEnabled
//
// if false a world context will render in front of everything else
//
void ContextContainer::SetDepthTestEnabled(T_ContextId const id, bool const depthEnabled)
{
	ContextData& ctxData = m_Contexts[id];
	ET_ASSERT(!ctxData.m_IsViewportContext, "Depth testing is only applicable on World contexts");

	m_WorldContexts[ctxData.m_Context].m_IsDepthEnabled = depthEnabled;
}

//------------------------------------
// ContextContainer::CreateDataModel
//
RefPtr<I_DataModel> ContextContainer::InstantiateDataModel(T_ContextId const id, core::HashString const modelId)
{
	return std::move(RmlGlobal::GetDataModelFactory().CreateModel(GetContext(id), modelId));
}

//------------------------------------
// ContextContainer::DestroyDataModel
//
bool ContextContainer::DestroyDataModel(T_ContextId const id, std::string const& modelName)
{
	return GetContext(id).DestroyDataModel(modelName);
}

//-------------------------------------
// ContextContainer::SetLoadedDocument
//
void ContextContainer::SetLoadedDocument(T_ContextId const id, core::HashString const documentId)
{
	ContextData& ctxData = m_Contexts[id];

	if (ctxData.m_IsViewportContext)
	{
		T_ViewportContexts::iterator const found = m_ViewportContexts.find(ctxData.m_Viewport);
		ET_ASSERT(found != m_ViewportContexts.cend());
		if (documentId.IsEmpty())
		{
			if (!found->second.m_Documents[ctxData.m_Context].IsEmpty())
			{
				found->second.m_Context.UnloadDocument(found->second.m_Documents[ctxData.m_Context]);
				found->second.m_Documents[ctxData.m_Context].Reset();
			}
		}
		else if (found->second.m_Documents[ctxData.m_Context].IsEmpty())
		{
			found->second.m_Context.LoadDocument(documentId);
			found->second.m_Documents[ctxData.m_Context] = documentId;
		}
	}
	else
	{
		Context& context = m_WorldContexts[ctxData.m_Context].m_Context;
		if (documentId.IsEmpty())
		{
			if (context.GetDocumentCount() > 0u)
			{
				context.UnloadDocument(context.GetDocumentId(0));
			}
		}
		else if (context.GetDocumentCount() == 0u)
		{
			context.LoadDocument(documentId);
		}
	}
}

#ifdef ET_DEBUG

//-------------------------------------
// ContextContainer::SetDebugContext
//
void ContextContainer::SetDebugContext(T_ContextId const id)
{
	if (id == m_DebuggerContext)
	{
		return;
	}

	if (id == INVALID_CONTEXT_ID)
	{
		Rml::Debugger::Shutdown();
	}
	else
	{
		Rml::Context* const context = GetContext(id).GetImpl();
		if (m_DebuggerContext == INVALID_CONTEXT_ID)
		{
			Rml::Debugger::Initialise(context);
			Rml::Debugger::SetVisible(m_IsDebuggerVisible);
		}
		else
		{
			Rml::Debugger::SetContext(context);
		}
	}

	m_DebuggerContext = id;
}

//--------------------------------------
// ContextContainer::SetDebuggerVisible
//
void ContextContainer::SetDebuggerVisible(bool const isVisible)
{
	m_IsDebuggerVisible = isVisible;
	if (m_DebuggerContext != INVALID_CONTEXT_ID)
	{
		Rml::Debugger::SetVisible(m_IsDebuggerVisible);
	}
}

#endif

//---------------------------------
// ContextContainer::GetContexts
//
// All contexts assigned to a viewport
//
Context* ContextContainer::GetContext(render::Viewport const* const vp) 
{
	T_ViewportContexts::iterator found = m_ViewportContexts.find(ToPtr(vp));
	if (found == m_ViewportContexts.cend())
	{
		return nullptr;
	}

	return &(found->second.m_Context);
}

//---------------------------------
// ContextContainer::GetContexts
//
Context* ContextContainer::GetContext(render::Viewport const* const vp, ContextRenderTarget*& renderTarget)
{
	T_ViewportContexts::iterator found = m_ViewportContexts.find(ToPtr(vp));
	if (found == m_ViewportContexts.cend())
	{
		renderTarget = nullptr;
		return nullptr;
	}

	renderTarget = &(found->second.m_RenderTarget);
	return &(found->second.m_Context);
}

//----------------------------------
// ContextContainer::GetContext
//
Rml::ElementDocument* ContextContainer::GetDocument(T_ContextId const id)
{
	ContextData& ctxData = m_Contexts[id];
	if (ctxData.m_IsViewportContext)
	{
		T_ViewportContexts::iterator const found = m_ViewportContexts.find(ctxData.m_Viewport);
		ET_ASSERT(found != m_ViewportContexts.cend());
		return found->second.m_Context.GetDocument(found->second.m_Documents[ctxData.m_Context]);
	}
	else
	{
		Context& context = m_WorldContexts[ctxData.m_Context].m_Context;
		return context.GetDocument(context.GetDocumentId(0u));
	}
}

//----------------------------------
// ContextContainer::GetContext
//
Context& ContextContainer::GetContext(T_ContextId const id)
{
	ContextData& ctxData = m_Contexts[id];

	if (ctxData.m_IsViewportContext)
	{
		T_ViewportContexts::iterator const found = m_ViewportContexts.find(ctxData.m_Viewport);
		ET_ASSERT(found != m_ViewportContexts.cend());
		return found->second.m_Context;
	}
	else
	{
		return m_WorldContexts[ctxData.m_Context].m_Context;
	}
}

//------------------------------------
// ContextContainer::OnViewportResize
//
void ContextContainer::OnViewportResize(render::Viewport const* const vp, ivec2 const dim)
{
	GetContext(vp)->SetDimensions(dim);
}

//-------------------------------------------
// ContextContainer::FindOrCreatePerViewport
//
ContextContainer::PerViewport& ContextContainer::FindOrCreatePerViewport(Ptr<render::Viewport> const viewport)
{
	std::pair<T_ViewportContexts::iterator, bool> found = m_ViewportContexts.try_emplace(viewport, PerViewport());
	PerViewport& perVp = found.first->second;
	if (found.second)
	{
		perVp.m_Viewport = viewport;
		perVp.m_ContextContainer = ToPtr(this);

		perVp.m_VPCallbackId = viewport->GetEventDispatcher().Register(render::E_ViewportEvent::VP_Resized, render::T_ViewportEventCallback(
			[this](render::T_ViewportEventFlags const, render::ViewportEventData const* const data) -> void
			{
				OnViewportResize(data->viewport, data->size);
			}));

		core::RawInputProvider* const inputProvider = viewport->GetInputProvider();
		if (inputProvider != nullptr)
		{
			inputProvider->RegisterListener(ToPtr(&perVp));
		}
	}
	else
	{
		ET_ASSERT(perVp.m_VPCallbackId != render::T_ViewportEventDispatcher::INVALID_ID);
	}

	return found.first->second;
}

//------------------------------------
// ContextContainer::ErasePerViewport
//
void ContextContainer::ErasePerViewport(render::Viewport* const vp, T_ViewportContexts::iterator const it)
{
	core::RawInputProvider* const inputProvider = vp->GetInputProvider();
	if (inputProvider != nullptr)
	{
		inputProvider->UnregisterListener(&(it->second));
	}

	ET_ASSERT(it->second.m_VPCallbackId != render::T_ViewportEventDispatcher::INVALID_ID);
	vp->GetEventDispatcher().Unregister(it->second.m_VPCallbackId);
	m_ViewportContexts.erase(it);
}


} // namespace gui
} // namespace et

