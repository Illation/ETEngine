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
	Rml::Input::KeyIdentifier const rmlKey = RmlUtil::GetRmlKeyId(key);
	if (rmlKey != Rml::Input::KeyIdentifier::KI_UNKNOWN)
	{
		int32 const mods = RmlUtil::GetRmlModifierFlags(modifiers);
		for (Context& context : m_Contexts)
		{
			if (context.IsActive() && context.IsDocumentLoaded())
			{
				if (context.ProcessKeyPressed(rmlKey, mods))
				{
					return true;
				}
			}
		}

		for (core::T_SlotId const worldContextId : m_EventWorldContexts)
		{
			WorldContext& worldContext = m_ContextContainer->GetWorldContexts()[worldContextId];
			if (worldContext.m_Context.IsActive() && worldContext.m_Context.IsDocumentLoaded())
			{
				if (worldContext.m_Context.ProcessKeyPressed(rmlKey, mods))
				{
					return true;
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
	Rml::Input::KeyIdentifier const rmlKey = RmlUtil::GetRmlKeyId(key);
	if (rmlKey != Rml::Input::KeyIdentifier::KI_UNKNOWN)
	{
		int32 const mods = RmlUtil::GetRmlModifierFlags(modifiers);
		for (Context& context : m_Contexts)
		{
			if (context.IsActive() && context.IsDocumentLoaded())
			{
				if (context.ProcessKeyReleased(rmlKey, mods))
				{
					return true;
				}
			}
		}

		for (core::T_SlotId const worldContextId : m_EventWorldContexts)
		{
			WorldContext& worldContext = m_ContextContainer->GetWorldContexts()[worldContextId];
			if (worldContext.m_Context.IsActive() && worldContext.m_Context.IsDocumentLoaded())
			{
				if (worldContext.m_Context.ProcessKeyReleased(rmlKey, mods))
				{
					return true;
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
	int32 const rmlButton = RmlUtil::GetRmlButtonIndex(button);
	if (rmlButton != -1)
	{
		int32 const mods = RmlUtil::GetRmlModifierFlags(modifiers);
		for (Context& context : m_Contexts)
		{
			if (context.IsActive() && context.IsDocumentLoaded())
			{
				if (context.ProcessMousePressed(rmlButton, mods))
				{
					return true;
				}
			}
		}

		for (core::T_SlotId const worldContextId : m_EventWorldContexts)
		{
			WorldContext& worldContext = m_ContextContainer->GetWorldContexts()[worldContextId];
			if (worldContext.m_Context.IsActive() && worldContext.m_Context.IsDocumentLoaded())
			{
				if (worldContext.m_Context.ProcessMousePressed(rmlButton, mods))
				{
					return true;
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
	int32 const rmlButton = RmlUtil::GetRmlButtonIndex(button);
	if (rmlButton != -1)
	{
		int32 const mods = RmlUtil::GetRmlModifierFlags(modifiers);
		for (Context& context : m_Contexts)
		{
			if (context.IsActive() && context.IsDocumentLoaded())
			{
				if (context.ProcessMouseReleased(rmlButton, mods))
				{
					return true;
				}
			}
		}

		for (core::T_SlotId const worldContextId : m_EventWorldContexts)
		{
			WorldContext& worldContext = m_ContextContainer->GetWorldContexts()[worldContextId];
			if (worldContext.m_Context.IsActive() && worldContext.m_Context.IsDocumentLoaded())
			{
				if (worldContext.m_Context.ProcessMouseReleased(rmlButton, mods))
				{
					return true;
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
	int32 const mods = RmlUtil::GetRmlModifierFlags(modifiers);
	for (Context& context : m_Contexts)
	{
		if (context.IsActive() && context.IsDocumentLoaded())
		{
			if (context.ProcessMouseMove(mousePos, mods))
			{
				return true;
			}
		}
	}

	render::Scene const* const renderScene = m_ContextContainer->GetRenderScene();
	for (core::T_SlotId const worldContextId : m_EventWorldContexts)
	{
		WorldContext& worldContext = m_ContextContainer->GetWorldContexts()[worldContextId];
		if (worldContext.m_Context.IsActive() && worldContext.m_Context.IsDocumentLoaded())
		{
			render::Camera const& camera = renderScene->GetCameras()[worldContext.m_EventCameraId];
			if (camera.GetViewport() == m_Viewport)
			{
				// transform the cursor into a world space ray from the camera through the near plane
				vec4 ndcMouse(((math::vecCast<float>(mousePos) * 2.f) / math::vecCast<float>(m_Viewport->GetDimensions())) - vec2(1.f), -1.f, 1.f);
				ndcMouse.y = -ndcMouse.y;
				vec4 rayEye = math::inverse(camera.GetProj()) * ndcMouse;
				//rayEye = vec4(rayEye.xy, -1.f, 0.f);
				vec4 const worldSpaceMouse = camera.GetViewInv() * rayEye;
				//vec4 const worldSpaceMouse = camera.GetViewProjInv() * ndcMouse;

				vec3 const r0 = camera.GetPosition();
				vec3 const rayDir = math::normalize(worldSpaceMouse.xyz - r0);

				// construct a rectangle on a plane from the contexts transform and dimensions
				vec2 const contextDim = math::vecCast<float>(worldContext.m_Context.GetDimensions());
				mat4 const transform = math::scale(vec3(contextDim, 1.f))
					* renderScene->GetNodes()[worldContext.m_NodeId];
				mat4 const orientation = math::DiscardW(transform);

				vec3 xTF = (orientation * vec4(vec3::RIGHT, 1.f)).xyz; // horizontal edge of the context rectangle
				vec3 yTF = (orientation * vec4(vec3::DOWN, 1.f)).xyz; // vertical edge of the context rectangle
				vec3 p0 = math::decomposePosition(transform) + (xTF * 0.5f) + (yTF * 0.5f); // top left corner of the rectangle
				vec3 pN = math::normalize(math::cross(xTF, yTF)); // rectangle plane normal

				// intersect the ray with the plane
				float dDotN = math::dot(rayDir, pN);
				if (!math::nearEquals(dDotN, 0.f))
				{
					//float pD = math::dot(pN, p0);
					//vec3 const intersection = (pD - math::dot(pN, r0)) / dDotN;
					float const a = math::dot(p0 - r0, pN) / dDotN;
					vec3 const intersection = r0 + (rayDir * a); // where the mouse hits the infinite plane

					// now project the difference vector of the intersection and rectangle origin onto the rectangle edges
					vec3 const p0p = intersection - p0;
					float const qx = math::dot(p0p, xTF) / math::lengthSquared(xTF);
					float const qy = math::dot(p0p, yTF) / math::lengthSquared(yTF);
					LOG(FS("screen: %s r0 %s rD %s intersection %s projection: %s", 
						mousePos.ToString().c_str(), 
						rayEye.ToString().c_str(),
						rayDir.ToString().c_str(),
						intersection.ToString().c_str(),
						vec2(qx, qy).ToString().c_str()));

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
	ivec2 const delta(mouseWheel.x, -mouseWheel.y);
	int32 const mods = RmlUtil::GetRmlModifierFlags(modifiers);
	for (Context& context : m_Contexts)
	{
		if (context.IsActive() && context.IsDocumentLoaded())
		{
			if (context.ProcessMouseWheelDelta(delta, mods))
			{
				return true;
			}
		}
	}

	for (core::T_SlotId const worldContextId : m_EventWorldContexts)
	{
		WorldContext& worldContext = m_ContextContainer->GetWorldContexts()[worldContextId];
		if (worldContext.m_Context.IsActive() && worldContext.m_Context.IsDocumentLoaded())
		{
			if (worldContext.m_Context.ProcessMouseWheelDelta(delta, mods))
			{
				return true;
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
	for (Context& context : m_Contexts)
	{
		if (context.IsActive() && context.IsDocumentLoaded())
		{
			if (context.ProcessTextInput(static_cast<Rml::Character>(character)))
			{
				return true;
			}
		}
	}

	for (core::T_SlotId const worldContextId : m_EventWorldContexts)
	{
		WorldContext& worldContext = m_ContextContainer->GetWorldContexts()[worldContextId];
		if (worldContext.m_Context.IsActive() && worldContext.m_Context.IsDocumentLoaded())
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


ContextContainer::T_Contexts ContextContainer::s_EmptyContexts;


//---------------------------------
// ContextContainer::OnTick
//
void ContextContainer::OnTick()
{
	// screen contexts
	for (auto& el : m_ViewportContexts)
	{
		for (Context& context : el.second.m_Contexts)
		{
			if (context.IsActive() && context.IsDocumentLoaded())
			{
				context.Update();
			}
		}
	}

	// world contexts
	for (WorldContext& worldContext : m_WorldContexts)
	{
		Context& context = worldContext.m_Context;
		if (context.IsActive() && context.IsDocumentLoaded())
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
	auto const context = perVp.m_Contexts.insert(Context());

	ContextData& ctxData = *ret.first;
	ctxData.m_Viewport = viewport;
	ctxData.m_Context = context.second;

	context.first->Init(std::to_string(ret.second), viewport->GetDimensions());
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

	if (ctxData.m_Viewport != nullptr)
	{
		T_ViewportContexts::iterator const found = m_ViewportContexts.find(ctxData.m_Viewport);
		ET_ASSERT(found != m_ViewportContexts.cend());
		if ((found->second.m_Contexts.size() == 1u) && (found->second.m_EventWorldContexts.empty()))
		{
			ErasePerViewport(ctxData.m_Viewport.Get(), found);
		}
		else
		{
			found->second.m_Contexts.erase(ctxData.m_Context);
		}
	}
	else
	{
		m_WorldContexts.erase(ctxData.m_Context);
	}

	m_Contexts.erase(id);
}

//------------------------------------
// ContextContainer::SetContextActive
//
void ContextContainer::SetContextActive(T_ContextId const id, bool const isActive)
{
	GetContext(id).SetActive(isActive);
}

//----------------------------------
// ContextContainer::SetEventCamera
//
// for 3D contexts set the camera we receive events from
// 
void ContextContainer::SetEventCamera(T_ContextId const id, core::T_SlotId const cameraId)
{
	ContextData& ctxData = m_Contexts[id];
	ET_ASSERT(ctxData.m_Viewport == nullptr, "Context event camera is only applicable on World contexts");

	WorldContext& worldContext = m_WorldContexts[ctxData.m_Context];
	if (worldContext.m_EventCameraId != cameraId)
	{
		if (worldContext.m_EventCameraId != core::INVALID_SLOT_ID)
		{
			render::Camera const& cam = m_RenderScene->GetCameras()[worldContext.m_EventCameraId];
			ET_ASSERT(cam.GetViewport() != nullptr);

			T_ViewportContexts::iterator found = m_ViewportContexts.find(cam.GetViewport());
			ET_ASSERT(found != m_ViewportContexts.cend());
			if ((found->second.m_EventWorldContexts.size() == 1u) && (found->second.m_Contexts.empty()))
			{
				ErasePerViewport(ctxData.m_Viewport.Get(), found);
			}
			else
			{
				found->second.m_EventWorldContexts.erase(ctxData.m_Context);
			}
		}

		worldContext.m_EventCameraId = cameraId;

		if (worldContext.m_EventCameraId != core::INVALID_SLOT_ID)
		{
			render::Camera const& cam = m_RenderScene->GetCameras()[worldContext.m_EventCameraId];
			ET_ASSERT(cam.GetViewport() != nullptr);

			PerViewport& perVp = FindOrCreatePerViewport(cam.GetViewport());
			perVp.m_EventWorldContexts.insert(ctxData.m_Context);
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
	ET_ASSERT(ctxData.m_Viewport == nullptr, "Context color is only applicable on World contexts");

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
	ET_ASSERT(ctxData.m_Viewport == nullptr, "Depth testing is only applicable on World contexts");

	m_WorldContexts[ctxData.m_Context].m_IsDepthEnabled = depthEnabled;
}

//------------------------------------
// ContextContainer::CreateDataModel
//
Rml::DataModelConstructor ContextContainer::CreateDataModel(T_ContextId const id, std::string const& modelName)
{
	return GetContext(id).CreateDataModel(modelName);
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
	Context& context = GetContext(id);
	if (documentId.IsEmpty())
	{
		if (context.IsDocumentLoaded())
		{
			context.UnloadDocument();
		}
	}
	else if (!context.IsDocumentLoaded())
	{
		context.LoadDocument(documentId);
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
ContextContainer::T_Contexts& ContextContainer::GetContexts(render::Viewport const* const vp) 
{
	T_ViewportContexts::iterator found = m_ViewportContexts.find(ToPtr(vp));
	if (found == m_ViewportContexts.cend())
	{
		return s_EmptyContexts;
	}

	return found->second.m_Contexts;
}

//---------------------------------
// ContextContainer::GetContexts
//
ContextContainer::T_Contexts& ContextContainer::GetContexts(render::Viewport const* const vp, ContextRenderTarget*& renderTarget)
{
	T_ViewportContexts::iterator found = m_ViewportContexts.find(ToPtr(vp));
	if (found == m_ViewportContexts.cend())
	{
		renderTarget = nullptr;
		return s_EmptyContexts;
	}

	renderTarget = &(found->second.m_RenderTarget);
	return found->second.m_Contexts;
}

//----------------------------------
// ContextContainer::GetContext
//
Rml::ElementDocument* ContextContainer::GetDocument(T_ContextId const id)
{
	return GetContext(id).GetDocument();
}

//----------------------------------
// ContextContainer::GetContext
//
Context& ContextContainer::GetContext(T_ContextId const id)
{
	ContextData& ctxData = m_Contexts[id];

	if (ctxData.m_Viewport != nullptr)
	{
		T_ViewportContexts::iterator const found = m_ViewportContexts.find(ctxData.m_Viewport);
		ET_ASSERT(found != m_ViewportContexts.cend());
		return found->second.m_Contexts[ctxData.m_Context];
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
	T_Contexts& contexts = GetContexts(vp);
	for (Context& context : contexts)
	{
		context.SetDimensions(dim);
	}
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

