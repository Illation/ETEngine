#pragma once
#include "Context.h"
#include "TickOrder.h"
#include "DataModel.h"

#include <EtCore/Containers/slot_map.h>
#include <EtCore/Input/RawInputListener.h>
#include <EtCore/UpdateCycle/Tickable.h>

#include <EtRendering/SceneStructure/RenderScene.h>

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

	struct ContextData
	{
		ContextData() = default;

		Ptr<render::Viewport> m_Viewport;
		core::T_SlotId m_Context;
		bool m_IsViewportContext = false;
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
		Ptr<render::Viewport const> m_Viewport;

		Context m_Context;
		core::slot_map<core::HashString> m_Documents;
		ContextRenderTarget m_RenderTarget;

		Ptr<ContextContainer> m_ContextContainer;
		render::T_ViewportEventCallbackId m_VPCallbackId = render::T_ViewportEventDispatcher::INVALID_ID;
		std::unordered_set<core::T_SlotId> m_EventWorldContexts;
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
		core::T_SlotId m_EventCameraId = core::INVALID_SLOT_ID;
		vec4 m_Color = vec4(1.f);
		bool m_IsDepthEnabled = true;
	};

	typedef core::slot_map<WorldContext> T_WorldContexts;

	// construct destruct
	//--------------------
	ContextContainer() : I_Tickable(static_cast<uint32>(E_TickOrder::TICK_ContextContainer)) {}
	ContextContainer(ContextContainer const&) = delete;
	void operator=(ContextContainer const&) = delete;

	~ContextContainer() = default;

	// init
	//------
	void SetRenderScene(Ptr<render::Scene const> const renderScene) { m_RenderScene = renderScene; }

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

	void SetEventCamera(T_ContextId const id, core::T_SlotId const cameraId);
	void SetContextColor(T_ContextId const id, vec4 const& color);
	void SetDepthTestEnabled(T_ContextId const id, bool const depthEnabled);

	RefPtr<I_DataModel> InstantiateDataModel(T_ContextId const id, core::HashString const modelId);
	bool DestroyDataModel(T_ContextId const id, std::string const& modelName);

	void SetLoadedDocument(T_ContextId const id, core::HashString const documentId);

	void SetInputEnabled(bool const enabled) { m_IsInputEnabled = enabled; }

#ifdef ET_DEBUG
	void SetDebugContext(T_ContextId const id);
	void SetDebuggerVisible(bool const isVisible);
#endif

	// accessors
	//-----------
	Context* GetContext(render::Viewport const* const vp);
	Context* GetContext(render::Viewport const* const vp, ContextRenderTarget*& renderTarget);
	T_WorldContexts& GetWorldContexts() { return m_WorldContexts; }

	Rml::ElementDocument* GetDocument(T_ContextId const id);

	render::Scene const* GetRenderScene() const { return m_RenderScene.Get(); }

	bool IsInputEnabled() const { return m_IsInputEnabled; }

#ifdef ET_DEBUG
	bool IsDebuggerVisible() const { return m_IsDebuggerVisible; }
#endif

	// utility
	//---------
private:
	Context& GetContext(T_ContextId const id);
	void OnViewportResize(render::Viewport const* const vp, ivec2 const dim);
	PerViewport& FindOrCreatePerViewport(Ptr<render::Viewport> const viewport);
	void ErasePerViewport(render::Viewport* const vp, T_ViewportContexts::iterator const it);

	// Data
	///////

private:
	core::slot_map<ContextData> m_Contexts;

	T_ViewportContexts m_ViewportContexts;
	T_WorldContexts m_WorldContexts;

	Ptr<render::Scene const> m_RenderScene;

	bool m_IsInputEnabled = true;

#ifdef ET_DEBUG
	T_ContextId m_DebuggerContext = INVALID_CONTEXT_ID;
	bool m_IsDebuggerVisible = false;
#endif
};


} // namespace gui
} // namespace et

