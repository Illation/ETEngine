#include "stdafx.h"
#include "GuiCanvasComponent.h"

#include <EtCore/Reflection/Registration.h>

#include <EtGUI/GuiExtension.h>
#include <EtGUI/Context/RmlGlobal.h>

#include <EtFramework/Components/TransformComponent.h>
#include <EtFramework/SceneGraph/UnifiedScene.h>


namespace et {
namespace fw {


// reflection
//------------

RTTR_REGISTRATION
{
	BEGIN_REGISTER_CLASS(GuiCanvasComponent, "gui canvas component")
		.property("GUI document asset", &GuiCanvasComponent::m_GuiDocumentId)
		.property("data model", &GuiCanvasComponent::m_DataModelId)
		.property("dimensions", &GuiCanvasComponent::m_Dimensions)
		.property("is active", &GuiCanvasComponent::m_IsActive)
		.property("depth test enabled", &GuiCanvasComponent::m_EnableDepthTest)
	END_REGISTER_CLASS_POLYMORPHIC(GuiCanvasComponent, I_ComponentDescriptor);
}

ECS_REGISTER_COMPONENT(GuiCanvasComponent);


//======================
// GUI Canvas Component 
//======================


//--------------------------------------
// GuiCanvasComponent::OnComponentAdded
//
// Register sprites in the render scene when they are added to the ECS
//
void GuiCanvasComponent::OnComponentAdded(EcsController& controller, GuiCanvasComponent& component, T_EntityId const entity)
{
	ET_ASSERT(component.m_Id == core::INVALID_SLOT_ID);

	gui::ContextContainer& contextContainer = UnifiedScene::Instance().GetGuiExtension()->GetContextContainer();

	component.m_Id = contextContainer.CreateContext(controller.GetComponent<TransformComponent>(entity).GetNodeId(), component.m_Dimensions);
	if (!component.m_DataModelId.IsEmpty())
	{
		component.m_DataModel = std::move(contextContainer.InstantiateDataModel(component.m_Id, component.m_DataModelId));
	}

	contextContainer.SetContextActive(component.m_Id, component.m_IsActive);
	contextContainer.SetLoadedDocument(component.m_Id, component.m_GuiDocumentId);
	contextContainer.SetDepthTestEnabled(component.m_Id, component.m_EnableDepthTest);
}

//--------------------------------------
// GuiCanvasComponent::OnComponentAdded
//
// Unregister sprites in the render scene when they are added to the ECS
//
void GuiCanvasComponent::OnComponentRemoved(EcsController& controller, GuiCanvasComponent& component, T_EntityId const entity)
{
	UNUSED(controller);
	UNUSED(entity);

	if (component.m_Id != core::INVALID_SLOT_ID)
	{
		gui::ContextContainer& contextContainer = UnifiedScene::Instance().GetGuiExtension()->GetContextContainer();
		if (!component.m_DataModelId.IsEmpty())
		{
			std::string modelName;
			gui::RmlGlobal::GetInstance()->GetDataModelFactory().GetModelName(component.m_DataModelId, modelName);
			contextContainer.DestroyDataModel(component.m_Id, modelName);
		}

		contextContainer.DestroyContext(component.m_Id);
	}
}

//---------------------------
// GuiCanvasComponent::c-tor
//
GuiCanvasComponent::GuiCanvasComponent(core::HashString const guiDocId,
	core::HashString const dataModelId, 
	ivec2 const dimensions, 
	vec2 const scale, 
	bool const isActive
)
	: m_GuiDocumentId(guiDocId)
	, m_DataModelId(dataModelId)
	, m_Dimensions(dimensions)
	, m_IsActive(isActive)
{}

//---------------------------------
// GuiCanvasComponent::SetDocument
//
void GuiCanvasComponent::SetDocument(core::HashString const guiDocId)
{
	m_GuiDocumentId = guiDocId;
	if (m_Id != core::INVALID_SLOT_ID)
	{
		UnifiedScene::Instance().GetGuiExtension()->GetContextContainer().SetLoadedDocument(m_Id, guiDocId);
	}
}

//---------------------------------
// GuiCanvasComponent::SetActive
//
void GuiCanvasComponent::SetActive(bool const isActive)
{
	m_IsActive = isActive;
	if (m_Id != core::INVALID_SLOT_ID)
	{
		UnifiedScene::Instance().GetGuiExtension()->GetContextContainer().SetContextActive(m_Id, isActive);
	}
}

//-----------------------------------------
// GuiCanvasComponent::SetDepthTestEnabled
//
void GuiCanvasComponent::SetDepthTestEnabled(bool const depthTest)
{
	m_EnableDepthTest = depthTest;
	if (m_Id != core::INVALID_SLOT_ID)
	{
		UnifiedScene::Instance().GetGuiExtension()->GetContextContainer().SetDepthTestEnabled(m_Id, m_EnableDepthTest);
	}
}


} // namespace fw
} // namespace et
