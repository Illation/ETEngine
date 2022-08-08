#pragma once
#include <EtCore/Containers/slot_map.h>
#include <EtCore/Input/RawInputListener.h>
#include <EtCore/UpdateCycle/Tickable.h>
#include <EtCore/Content/AssetPointer.h>
#include <EtCore/Containers/slot_map.h>

#include <EtRendering/Extensions/SceneExtension.h>
#include <EtRendering/SceneStructure/NodeIdFwd.h>
#include <EtRendering/SceneStructure/RenderScene.h>

#include <EtGUI/Context/Context.h>
#include <EtGUI/Context/TickOrder.h>
#include <EtGUI/Context/DataModel.h>
#include <EtGUI/Rendering/ContextRenderTarget.h>


// fwd
namespace et { namespace rendering {
	class TextureData;
	class Viewport;
} }


namespace et {
namespace gui {


//---------------------------------
// GuiExtension
//
// Scene render data for UI
//
class GuiExtension final : public render::I_SceneExtension, public core::I_Tickable
{
	// definitions
	//-------------
public:
	static core::HashString const s_ExtensionId;

private:

	//---------------------------------
	// ContextData
	//
	// Link either to a PerViewport or a WorldContext
	//
	struct ContextData
	{
		ContextData() = default;

		Ptr<rhi::Viewport> m_Viewport;
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
		Ptr<rhi::Viewport const> m_Viewport;

		Context m_Context;
		core::slot_map<core::HashString> m_Documents;
		ContextRenderTarget m_RenderTarget;

		Ptr<GuiExtension> m_GuiExtension;
		rhi::T_ViewportEventCallbackId m_VPCallbackId = rhi::T_ViewportEventDispatcher::INVALID_ID;
		std::unordered_set<core::T_SlotId> m_EventWorldContexts;
	};

	typedef std::unordered_map<Ptr<rhi::Viewport const>, PerViewport> T_ViewportContexts;

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
	GuiExtension() : I_SceneExtension(), I_Tickable(static_cast<uint32>(E_TickOrder::TICK_GuiExtension)) {}

	GuiExtension(GuiExtension const&) = delete;
	void operator=(GuiExtension const&) = delete;

	~GuiExtension() = default;

	// init
	//------
	void SetRenderScene(Ptr<render::Scene const> const renderScene) { m_RenderScene = renderScene; }

	// I_SceneExtension interface
	//----------------------------
	core::HashString GetId() const override { return s_ExtensionId; }

	// tickable interface
	//--------------------
protected:
	void OnTick() override;

	// functionality
	//---------------
public:
	T_ContextId CreateContext(Ptr<rhi::Viewport> const viewport);
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

	// accessors
	//-----------
	Context* GetContext(rhi::Viewport const* const vp);
	Context* GetContext(rhi::Viewport const* const vp, ContextRenderTarget*& renderTarget);
	T_WorldContexts& GetWorldContexts() { return m_WorldContexts; }

	Rml::ElementDocument* GetDocument(T_ContextId const id);

	render::Scene const* GetRenderScene() const { return m_RenderScene.Get(); }

	bool IsInputEnabled() const { return m_IsInputEnabled; }

	// utility
	//---------
private:
	Context& GetContext(T_ContextId const id);
	void OnViewportResize(rhi::Viewport const* const vp, ivec2 const dim);
	PerViewport& FindOrCreatePerViewport(Ptr<rhi::Viewport> const viewport);
	void ErasePerViewport(rhi::Viewport* const vp, T_ViewportContexts::iterator const it);


	// Data
	///////

	core::slot_map<ContextData> m_Contexts;

	T_ViewportContexts m_ViewportContexts;
	T_WorldContexts m_WorldContexts;

	Ptr<render::Scene const> m_RenderScene;

	bool m_IsInputEnabled = true;
};


} // namespace gui
} // namespace et
