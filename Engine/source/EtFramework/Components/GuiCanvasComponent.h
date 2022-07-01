#pragma once
#include <EtFramework/ECS/EcsController.h>
#include <EtFramework/SceneGraph/ComponentDescriptor.h>
#include <EtFramework/SceneGraph/EntityLink.h>

#include <EtGUI/Context/DataModel.h>


namespace et {
REGISTRATION_NS(fw);
}


namespace et {
namespace fw {


//---------------------------------
// GuiCanvasComponent
//
// Provides a surface to place GUI documents in
//
class GuiCanvasComponent final : public SimpleComponentDescriptor
{
	// definitions
	//-------------
	ECS_DECLARE_COMPONENT

	RTTR_ENABLE(SimpleComponentDescriptor) // for serialization
	REGISTRATION_FRIEND_NS(fw)

public:
	//---------------------------------
	// E_RenderMode
	//
	enum class E_RenderMode 
	{
		ScreenSpaceOverlay, // the canvas is rendered on top of everything after post processing is applied
		WorldSpace, // the canvas is rendered into the world before post processing is applied

		Invalid
	};

	// construct destruct
	//--------------------
private:
	GuiCanvasComponent() = default;
public:
	GuiCanvasComponent(E_RenderMode const renderMode,
		core::HashString const guiDocId, 
		core::HashString const dataModelId, 
		ivec2 const dimensions, 
		vec4 const& color = vec4(1.f),
		vec2 const scale = vec2(1.f), 
		bool const isActive = true);
	~GuiCanvasComponent() = default;

	// init deinit
	//-------------

	static void OnComponentAdded(EcsController& controller, GuiCanvasComponent& component, T_EntityId const entity);
	static void OnComponentRemoved(EcsController& controller, GuiCanvasComponent& component, T_EntityId const entity);

	// interface
	//-----------
	bool CallScenePostLoad() const override { return true; }
	void OnScenePostLoad(EcsController& ecs, T_EntityId const id) override;

	// accessors
	//-----------
	gui::T_ContextId GetId() const { return m_Id; }
	gui::I_DataModel const* GetDataModel() const { return m_DataModel.Get(); }
	gui::I_DataModel* GetDataModel() { return m_DataModel.Get(); }
	E_RenderMode GetRenderMode() const { return m_RenderMode; }
	bool IsActive() const { return m_IsActive; }
	core::HashString GetGuiDocumentId() const { return m_GuiDocumentId; }

	// for world space contexts
	ivec2 GetDimensions() const { return m_Dimensions; }
	vec4 const& GetColor() const { return m_Color; }
	bool IsDepthTestEnabled() const { return m_EnableDepthTest; }

	// modifiers
	//-----------
	void SetActive(bool const isActive);
	void SetDocument(core::HashString const guiDocId);

	// for world space contexts
	void SetCamera(T_EntityId const cameraEntity);
	void SetColor(vec4 const& color);
	void SetDepthTestEnabled(bool const depthTest);

	// utility
	//-----------
private:
	void InitForScreenSpace(EcsController const& ecs);
	void UpdateCamera();

	// Data
	///////

	gui::T_ContextId m_Id = gui::INVALID_CONTEXT_ID;
	RefPtr<gui::I_DataModel> m_DataModel;

	// reflected
	E_RenderMode m_RenderMode = E_RenderMode::Invalid;
	bool m_IsActive = true;
	core::HashString m_GuiDocumentId;
	core::HashString m_DataModelId;
	EntityLink m_Camera; // for screenspace canvases defines the viewport to render to, for worldspace canvases defines where the events are coming from

	// for world space contexts
	ivec2 m_Dimensions;
	vec4 m_Color = vec4(1.f);
	bool m_EnableDepthTest = true;
};

class GuiCanvasComponentLinkEnforcer final
{
	DECLARE_FORCED_LINKING()
};


} // namespace fw
} // namespace et
