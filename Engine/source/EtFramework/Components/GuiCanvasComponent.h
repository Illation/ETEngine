#pragma once
#include <EtFramework/ECS/EcsController.h>
#include <EtFramework/SceneGraph/ComponentDescriptor.h>

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
		vec2 const scale = vec2(1.f), 
		bool const isActive = true);
	~GuiCanvasComponent() = default;

	// init deinit
	//-------------

	static void OnComponentAdded(EcsController& controller, GuiCanvasComponent& component, T_EntityId const entity);
	static void OnComponentRemoved(EcsController& controller, GuiCanvasComponent& component, T_EntityId const entity);

	// accessors
	//-----------
	gui::I_DataModel const* GetDataModel() const { return m_DataModel.Get(); }
	gui::I_DataModel* GetDataModel() { return m_DataModel.Get(); }
	ivec2 GetDimensions() const { return m_Dimensions; }
	bool IsActive() const { return m_IsActive; }

	// modifiers
	//-----------
	void SetDocument(core::HashString const guiDocId);
	void SetActive(bool const isActive);

	// Data
	///////

private:
	core::T_SlotId m_Id = core::INVALID_SLOT_ID;
	RefPtr<gui::I_DataModel> m_DataModel;

	core::HashString m_GuiDocumentId;
	core::HashString m_DataModelId;
	ivec2 m_Dimensions;
	bool m_IsActive = true;
};


} // namespace fw
} // namespace et
