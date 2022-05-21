#pragma once
#include "Context.h"
#include "TickOrder.h"

#include <EtCore/Containers/slot_map.h>
#include <EtCore/Input/RawInputListener.h>
#include <EtCore/UpdateCycle/Tickable.h>


// fwd
namespace et { namespace render {
	class Viewport;
} }


namespace et {
namespace gui {


//---------------------------------
// ContextContainer
//
// Root data storage for a collection of UI contexts
//
class ContextContainer : public core::I_Tickable
{
	// definitions
	//-------------
public:
	typedef core::slot_map<Context> T_Contexts;

private:

	static T_Contexts const s_EmptyContexts;

	struct ContextData
	{
		ContextData() = default;

		Ptr<render::Viewport> m_Viewport;
		core::T_SlotId m_Context;
	};

	//---------------------------------
	// PerViewport
	//
	// Contains all contexts associated with a specific viewport
	//
	struct PerViewport final : public core::I_RawInputListener
	{
		PerViewport() = default;

		// interface
		int8 GetPriority() const override { return 1; }
		bool ProcessKeyPressed(E_KbdKey const key) override;
		bool ProcessKeyReleased(E_KbdKey const key) override;
		bool ProcessMousePressed(E_MouseButton const button) override;
		bool ProcessMouseReleased(E_MouseButton const button) override;
		bool ProcessMouseMove(ivec2 const& mousePos) override;
		bool ProcessMouseWheelDelta(ivec2 const& mouseWheel) override;

		// data
		T_Contexts m_Contexts;
		render::T_ViewportEventCallbackId m_VPCallbackId = render::T_ViewportEventDispatcher::INVALID_ID;
	};

	typedef std::unordered_map<Ptr<render::Viewport const>, PerViewport> T_ViewportContexts;

	// construct destruct
	//--------------------
public:
	ContextContainer() : I_Tickable(static_cast<uint32>(E_TickOrder::TICK_ContextContainer)) {}
	ContextContainer(ContextContainer const&) = delete;
	void operator=(ContextContainer const&) = delete;

	~ContextContainer() = default;

	// tickable interface
	//--------------------
protected:
	void OnTick() override;

	// functionality
	//---------------
public:
	T_ContextId CreateContext(Ptr<render::Viewport> const viewport);
	void SetContextActive(T_ContextId const id, bool const isActive);
	void SetLoadedDocument(T_ContextId const id, core::HashString const documentId);
	void DestroyContext(T_ContextId const id);

	// accessors
	//-----------
	T_Contexts& GetContexts(render::Viewport const* const vp);
	T_Contexts const& GetContexts(render::Viewport const* const vp) const;

	// utility
	//---------
private:
	Context& GetContext(T_ContextId const id);
	void OnViewportResize(render::Viewport const* const vp, ivec2 const dim);

	// Data
	///////

private:
	core::slot_map<ContextData> m_Contexts;
	T_ViewportContexts m_ViewportContexts;
};


} // namespace gui
} // namespace et

