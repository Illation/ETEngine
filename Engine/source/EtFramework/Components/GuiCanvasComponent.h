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
// In world Canvas provides a surface to draw UI on
//
class GuiCanvasComponent final : public SimpleComponentDescriptor
{
	// definitions
	//-------------
	ECS_DECLARE_COMPONENT

	RTTR_ENABLE(SimpleComponentDescriptor) // for serialization
	REGISTRATION_FRIEND_NS(fw)

	// construct destruct
	//--------------------
	GuiCanvasComponent() = default;
public:
	GuiCanvasComponent(core::HashString const guiDocId, 
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
	gui::I_DataModel const* GetDataModel() const { return m_DataModel.Get(); }
	gui::I_DataModel* GetDataModel() { return m_DataModel.Get(); }
	ivec2 GetDimensions() const { return m_Dimensions; }
	vec4 const& GetColor() const { return m_Color; }
	bool IsActive() const { return m_IsActive; }
	bool IsDepthTestEnabled() const { return m_EnableDepthTest; }

	// modifiers
	//-----------
	void SetDocument(core::HashString const guiDocId);
	void SetEventCamera(T_EntityId const cameraEntity);
	void SetColor(vec4 const& color);
	void SetActive(bool const isActive);
	void SetDepthTestEnabled(bool const depthTest);

	// utility
	//-----------
private:
	void UpdateEventCamera();

	// Data
	///////

	core::T_SlotId m_Id = core::INVALID_SLOT_ID;
	RefPtr<gui::I_DataModel> m_DataModel;

	// reflected
	core::HashString m_GuiDocumentId;
	core::HashString m_DataModelId;
	EntityLink m_EventCamera;
	ivec2 m_Dimensions;
	vec4 m_Color = vec4(1.f);
	bool m_IsActive = true;
	bool m_EnableDepthTest = true;
};


} // namespace fw
} // namespace et
