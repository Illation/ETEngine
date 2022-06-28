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

	std::pair<T_ViewportContexts::iterator, bool> found = m_ViewportContexts.try_emplace(viewport, PerViewport());
	PerViewport& perVp = found.first->second;
	if (found.second)
	{
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
		if (found->second.m_Contexts.size() == 1u)
		{
			core::RawInputProvider* const inputProvider = ctxData.m_Viewport->GetInputProvider();
			if (inputProvider != nullptr)
			{
				inputProvider->UnregisterListener(&(found->second));
			}

			ET_ASSERT(found->second.m_VPCallbackId != render::T_ViewportEventDispatcher::INVALID_ID);
			ctxData.m_Viewport->GetEventDispatcher().Unregister(found->second.m_VPCallbackId);
			m_ViewportContexts.erase(found);
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
	return std::move(RmlGlobal::GetInstance()->GetDataModelFactory().CreateModel(GetContext(id), modelId));
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


} // namespace gui
} // namespace et

