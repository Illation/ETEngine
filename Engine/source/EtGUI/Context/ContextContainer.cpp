#include "stdafx.h"
#include "ContextContainer.h"

#include <EtCore/Input/RawInputProvider.h>

#include <EtRendering/GraphicsContext/Viewport.h>


namespace et {
namespace gui {


//===================================
// Context Container :: Per Viewport
//===================================


//--------------------------------
// PerViewport::ProcessKeyPressed
//
bool ContextContainer::PerViewport::ProcessKeyPressed(E_KbdKey const key)
{
	for (Context& context : m_Contexts)
	{
		if (context.IsActive() && context.IsDocumentLoaded())
		{
			if (context.ProcessKeyPressed(key))
			{
				return true;
			}
		}
	}

	return false;
}

//---------------------------------
// PerViewport::ProcessKeyReleased
//
bool ContextContainer::PerViewport::ProcessKeyReleased(E_KbdKey const key)
{
	for (Context& context : m_Contexts)
	{
		if (context.IsActive() && context.IsDocumentLoaded())
		{
			if (context.ProcessKeyReleased(key))
			{
				return true;
			}
		}
	}

	return false;
}

//----------------------------------
// PerViewport::ProcessMousePressed
//
bool ContextContainer::PerViewport::ProcessMousePressed(E_MouseButton const button)
{
	for (Context& context : m_Contexts)
	{
		if (context.IsActive() && context.IsDocumentLoaded())
		{
			if (context.ProcessMousePressed(button))
			{
				return true;
			}
		}
	}

	return false;
}

//-----------------------------------
// PerViewport::ProcessMouseReleased
//
bool ContextContainer::PerViewport::ProcessMouseReleased(E_MouseButton const button)
{
	for (Context& context : m_Contexts)
	{
		if (context.IsActive() && context.IsDocumentLoaded())
		{
			if (context.ProcessMouseReleased(button))
			{
				return true;
			}
		}
	}

	return false;
}

//-------------------------------
// PerViewport::ProcessMouseMove
//
bool ContextContainer::PerViewport::ProcessMouseMove(ivec2 const& mousePos)
{
	for (Context& context : m_Contexts)
	{
		if (context.IsActive() && context.IsDocumentLoaded())
		{
			if (context.ProcessMouseMove(mousePos))
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
bool ContextContainer::PerViewport::ProcessMouseWheelDelta(ivec2 const& mouseWheel)
{
	for (Context& context : m_Contexts)
	{
		if (context.IsActive() && context.IsDocumentLoaded())
		{
			if (context.ProcessMouseWheelDelta(mouseWheel))
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


ContextContainer::T_Contexts const ContextContainer::s_EmptyContexts;


//---------------------------------
// ContextContainer::CreateContext
//
// May create a new context list and bind for viewport resize events
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

//------------------------------------
// ContextContainer::SetContextActive
//
void ContextContainer::SetContextActive(T_ContextId const id, bool const isActive)
{
	GetContext(id).SetActive(isActive);
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

//----------------------------------
// ContextContainer::DestroyContext
//
// will delete the last PerViewport data and event bindings
//
void ContextContainer::DestroyContext(T_ContextId const id)
{
	ContextData& ctxData = m_Contexts[id];

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

	m_Contexts.erase(id);
}

//---------------------------------
// ContextContainer::GetContexts
//
// All contexts assigned to a viewport
//
ContextContainer::T_Contexts& ContextContainer::GetContexts(render::Viewport const* const vp) 
{
	T_ViewportContexts::iterator found = m_ViewportContexts.find(ToPtr(vp));
	ET_ASSERT(found != m_ViewportContexts.cend());

	return found->second.m_Contexts;
}

//---------------------------------
// ContextContainer::GetContexts
//
ContextContainer::T_Contexts const& ContextContainer::GetContexts(render::Viewport const* const vp) const
{
	T_ViewportContexts::const_iterator const found = m_ViewportContexts.find(ToPtr(vp));
	if (found == m_ViewportContexts.cend())
	{
		return s_EmptyContexts;
	}

	return found->second.m_Contexts;
}

//----------------------------------
// ContextContainer::GetContext
//
Context& ContextContainer::GetContext(T_ContextId const id)
{
	ContextData& ctxData = m_Contexts[id];
	T_ViewportContexts::iterator const found = m_ViewportContexts.find(ctxData.m_Viewport);
	ET_ASSERT(found != m_ViewportContexts.cend());
	return found->second.m_Contexts[ctxData.m_Context];
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

