#include "stdafx.h"
#include "ContextContainer.h"

#include <EtRendering/GraphicsContext/Viewport.h>


namespace et {
namespace gui {


//===================
// Context Container
//===================


ContextContainer::T_Contexts const ContextContainer::s_EmptyContexts;


//---------------------------------
// ContextContainer::CreateContext
//
// May create a new context list and bind for viewport resize events
//
core::T_SlotId ContextContainer::CreateContext(Ptr<render::Viewport> const viewport)
{
	auto const ret = m_Contexts.insert(ContextData());

	std::pair<T_ViewportContexts::iterator, bool> found = m_ViewportContexts.try_emplace(viewport, PerViewport());
	PerViewport& perVp = found.first->second;
	if (!found.second)
	{
		perVp.m_VPCallbackId = viewport->GetEventDispatcher().Register(render::E_ViewportEvent::VP_Resized, render::T_ViewportEventCallback(
			[this](render::T_ViewportEventFlags const, render::ViewportEventData const* const data) -> void
			{
				OnViewportResize(data->viewport, data->size);
			}));
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
void ContextContainer::SetContextActive(core::T_SlotId const contextId, bool const isActive)
{
	GetContext(contextId).SetActive(isActive);
}

//----------------------------------
// ContextContainer::DestroyContext
//
// will delete the last PerViewport data and event bindings
//
void ContextContainer::DestroyContext(core::T_SlotId const contextId)
{
	ContextData& ctxData = m_Contexts[contextId];

	T_ViewportContexts::iterator const found = m_ViewportContexts.find(ctxData.m_Viewport);
	ET_ASSERT(found != m_ViewportContexts.cend());
	if (found->second.m_Contexts.size() == 1u)
	{
		ET_ASSERT(found->second.m_VPCallbackId != render::T_ViewportEventDispatcher::INVALID_ID);
		ctxData.m_Viewport->GetEventDispatcher().Unregister(found->second.m_VPCallbackId);
		m_ViewportContexts.erase(found);
	}
	else
	{
		found->second.m_Contexts.erase(ctxData.m_Context);
	}

	m_Contexts.erase(contextId);
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
Context& ContextContainer::GetContext(core::T_SlotId const contextId)
{
	ContextData& ctxData = m_Contexts[contextId];
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

