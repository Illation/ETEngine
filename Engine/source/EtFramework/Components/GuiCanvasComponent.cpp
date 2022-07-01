#include "stdafx.h"
#include "GuiCanvasComponent.h"

#include "CameraComponent.h"

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
		.property("is active", &GuiCanvasComponent::m_IsActive)
		.property("GUI document asset", &GuiCanvasComponent::m_GuiDocumentId)
		.property("data model", &GuiCanvasComponent::m_DataModelId)
		.property("event camera", &GuiCanvasComponent::m_EventCamera)
		.property("dimensions", &GuiCanvasComponent::m_Dimensions)
		.property("color", &GuiCanvasComponent::m_Color)
		.property("depth test enabled", &GuiCanvasComponent::m_EnableDepthTest)
	END_REGISTER_CLASS_POLYMORPHIC(GuiCanvasComponent, I_ComponentDescriptor);
}

ECS_REGISTER_COMPONENT(GuiCanvasComponent);


//======================
// GUI Canvas Component 
//======================


//---------------------------
// GuiCanvasComponent::c-tor
//
GuiCanvasComponent::GuiCanvasComponent(core::HashString const guiDocId,
	core::HashString const dataModelId,
	ivec2 const dimensions,
	vec4 const& color,
	vec2 const scale,
	bool const isActive
)
	: m_GuiDocumentId(guiDocId)
	, m_DataModelId(dataModelId)
	, m_Dimensions(dimensions)
	, m_Color(color)
	, m_IsActive(isActive)
{}

//--------------------------------------
// GuiCanvasComponent::OnComponentAdded
//
// Register the canvas in the GUI extension when the component is added to the ECS
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
	contextContainer.SetContextColor(component.m_Id, component.m_Color);
	contextContainer.SetDepthTestEnabled(component.m_Id, component.m_EnableDepthTest);
}

//--------------------------------------
// GuiCanvasComponent::OnComponentAdded
//
// Unregister the canvas from the GUI extension when the component is removed to the ECS
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
			gui::RmlGlobal::GetDataModelFactory().GetModelName(component.m_DataModelId, modelName);
			contextContainer.DestroyDataModel(component.m_Id, modelName);
		}

		contextContainer.DestroyContext(component.m_Id);
	}
}

//-------------------------------------
// GuiCanvasComponent::OnScenePostLoad
//
void GuiCanvasComponent::OnScenePostLoad(EcsController& ecs, T_EntityId const id)
{
	UNUSED(ecs);
	UNUSED(id);
	ET_ASSERT(m_Id != core::INVALID_SLOT_ID);
	UpdateEventCamera();
}

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

//------------------------------------
// GuiCanvasComponent::SetEventCamera
//
void GuiCanvasComponent::SetEventCamera(T_EntityId const cameraEntity)
{
	m_EventCamera.SetId(cameraEntity);
	if (m_Id != core::INVALID_SLOT_ID)
	{
		UpdateEventCamera();
	}
}

//---------------------------------
// GuiCanvasComponent::SetColor
//
void GuiCanvasComponent::SetColor(vec4 const& color)
{
	m_Color = color;
	if (m_Id != core::INVALID_SLOT_ID)
	{
		UnifiedScene::Instance().GetGuiExtension()->GetContextContainer().SetContextColor(m_Id, m_Color);
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

//---------------------------------------
// GuiCanvasComponent::UpdateEventCamera
//
// Link up the event camera once the entity link is resolved
//
void GuiCanvasComponent::UpdateEventCamera()
{
	UnifiedScene& uniScene = UnifiedScene::Instance();
	if (m_EventCamera.GetId() != INVALID_ENTITY_ID)
	{
		EcsController& ecs = uniScene.GetEcs();

		ET_ASSERT(ecs.HasComponent<CameraComponent>(m_EventCamera.GetId()));
		CameraComponent const& eventCamera = ecs.GetComponent<CameraComponent>(m_EventCamera.GetId());

		ET_ASSERT(m_EventCamera.GetId() != core::INVALID_SLOT_ID);
		uniScene.GetGuiExtension()->GetContextContainer().SetEventCamera(m_Id, eventCamera.GetId());
	}
	else
	{
		uniScene.GetGuiExtension()->GetContextContainer().SetEventCamera(m_Id, core::INVALID_SLOT_ID);
	}
}


} // namespace fw
} // namespace et
