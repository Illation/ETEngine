#pragma once
#include "Context.h"
#include "TickOrder.h"
#include "DataModel.h"

#include <EtCore/Containers/slot_map.h>
#include <EtCore/Input/RawInputListener.h>
#include <EtCore/UpdateCycle/Tickable.h>

#include <EtGUI/Rendering/ContextRenderTarget.h>


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

	static T_Contexts s_EmptyContexts;

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
		bool ProcessKeyPressed(E_KbdKey const key, core::T_KeyModifierFlags const modifiers) override;
		bool ProcessKeyReleased(E_KbdKey const key, core::T_KeyModifierFlags const modifiers) override;
		bool ProcessMousePressed(E_MouseButton const button, core::T_KeyModifierFlags const modifiers) override;
		bool ProcessMouseReleased(E_MouseButton const button, core::T_KeyModifierFlags const modifiers) override;
		bool ProcessMouseMove(ivec2 const& mousePos, core::T_KeyModifierFlags const modifiers) override;
		bool ProcessMouseWheelDelta(ivec2 const& mouseWheel, core::T_KeyModifierFlags const modifiers) override;
		bool ProcessTextInput(core::E_Character const character) override;

		// data
		T_Contexts m_Contexts;
		ContextRenderTarget m_RenderTarget;
		render::T_ViewportEventCallbackId m_VPCallbackId = render::T_ViewportEventDispatcher::INVALID_ID;
	};

	typedef std::unordered_map<Ptr<render::Viewport const>, PerViewport> T_ViewportContexts;

public:
	//---------------------------------
	// WorldContext
	//
	// Data for a context tied to a specific world transform
	//
	struct WorldContext
	{
		Context m_Context;
		ContextRenderTarget m_RenderTarget;
		core::T_SlotId m_NodeId;
	};

	typedef core::slot_map<WorldContext> T_WorldContexts;

	// construct destruct
	//--------------------
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
	T_ContextId CreateContext(core::T_SlotId const nodeId, ivec2 const dimensions);
	void DestroyContext(T_ContextId const id);

	void SetContextActive(T_ContextId const id, bool const isActive);

	Rml::DataModelConstructor CreateDataModel(T_ContextId const id, std::string const& modelName);
	RefPtr<I_DataModel> InstantiateDataModel(T_ContextId const id, core::HashString const modelId);
	bool DestroyDataModel(T_ContextId const id, std::string const& modelName);

	void SetLoadedDocument(T_ContextId const id, core::HashString const documentId);

#ifdef ET_DEBUG
	void SetDebugContext(T_ContextId const id);
	void SetDebuggerVisible(bool const isVisible);
#endif

	// accessors
	//-----------
	T_Contexts& GetContexts(render::Viewport const* const vp);
	T_Contexts& GetContexts(render::Viewport const* const vp, ContextRenderTarget*& renderTarget);
	T_WorldContexts& GetWorldContexts() { return m_WorldContexts; }

	Rml::ElementDocument* GetDocument(T_ContextId const id);

#ifdef ET_DEBUG
	bool IsDebuggerVisible() const { return m_IsDebuggerVisible; }
#endif

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
	T_WorldContexts m_WorldContexts;

#ifdef ET_DEBUG
	T_ContextId m_DebuggerContext = INVALID_CONTEXT_ID;
	bool m_IsDebuggerVisible = false;
#endif
};


} // namespace gui
} // namespace et

