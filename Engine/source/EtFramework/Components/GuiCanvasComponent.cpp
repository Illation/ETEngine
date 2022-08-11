#include "stdafx.h"
#include "GuiCanvasComponent.h"

#include "CameraComponent.h"

#include <EtCore/Reflection/Registration.h>

#include <EtGUI/Context/RmlGlobal.h>

#include <EtFramework/Components/TransformComponent.h>
#include <EtFramework/GUI/GuiExtension.h>
#include <EtFramework/SceneGraph/UnifiedScene.h>


namespace et {
namespace fw {


//======================
// GUI Canvas Component 
//======================


// reflection
//------------

RTTR_REGISTRATION
{
	rttr::registration::enumeration<GuiCanvasComponent::E_RenderMode>("gui canvas render mode") (
		rttr::value("Screen Space Overlay", GuiCanvasComponent::E_RenderMode::ScreenSpaceOverlay),
		rttr::value("World Space", GuiCanvasComponent::E_RenderMode::WorldSpace));

	BEGIN_REGISTER_CLASS(GuiCanvasComponent, "gui canvas component")
		.property("render mode", &GuiCanvasComponent::m_RenderMode)
		.property("is active", &GuiCanvasComponent::m_IsActive)
		.property("GUI document asset", &GuiCanvasComponent::m_GuiDocumentId)
		.property("data model", &GuiCanvasComponent::m_DataModelId)
		.property("camera", &GuiCanvasComponent::m_Camera)
		.property("dimensions", &GuiCanvasComponent::m_Dimensions)
		.property("color", &GuiCanvasComponent::m_Color)
		.property("depth test enabled", &GuiCanvasComponent::m_EnableDepthTest)
	END_REGISTER_CLASS_POLYMORPHIC(GuiCanvasComponent, I_ComponentDescriptor);
}

ECS_REGISTER_COMPONENT(GuiCanvasComponent);

DEFINE_FORCED_LINKING(GuiCanvasComponentLinkEnforcer)


//---------------------------
// GuiCanvasComponent::c-tor
//
GuiCanvasComponent::GuiCanvasComponent(E_RenderMode const renderMode, 
	core::HashString const guiDocId,
	core::HashString const dataModelId,
	ivec2 const dimensions,
	vec4 const& color,
	vec2 const scale,
	bool const isActive
)
	: m_RenderMode(renderMode)
	, m_GuiDocumentId(guiDocId)
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
	if (component.m_RenderMode == E_RenderMode::WorldSpace)
	{
		ET_ASSERT(component.m_Id == gui::INVALID_CONTEXT_ID);

		GuiExtension& guiExt = *UnifiedScene::Instance().GetGuiExtension();

		component.m_Id = guiExt.CreateContext(controller.GetComponent<TransformComponent>(entity).GetNodeId(), component.m_Dimensions);
		if (!component.m_DataModelId.IsEmpty())
		{
			component.m_DataModel = std::move(guiExt.InstantiateDataModel(component.m_Id, component.m_DataModelId));
		}

		guiExt.SetLoadedDocument(component.m_Id, component.m_GuiDocumentId);
		guiExt.SetContextActive(component.m_Id, component.m_IsActive);
		guiExt.SetContextColor(component.m_Id, component.m_Color);
		guiExt.SetDepthTestEnabled(component.m_Id, component.m_EnableDepthTest);
	}
	else if (component.m_RenderMode == E_RenderMode::ScreenSpaceOverlay)
	{
		if (UnifiedScene::Instance().IsSceneLoaded())
		{
			component.InitForScreenSpace(controller);
		}
	}
}

//--------------------------------------
// GuiCanvasComponent::OnComponentAdded
//
// Unregister the canvas from the GUI extension when the component is removed to the ECS
//
void GuiCanvasComponent::OnComponentRemoved(EcsController& controller, GuiCanvasComponent& component, T_EntityId const entity)
{
	ET_UNUSED(controller);
	ET_UNUSED(entity);

	if (component.m_Id != gui::INVALID_CONTEXT_ID)
	{
		GuiExtension& guiExt = *UnifiedScene::Instance().GetGuiExtension();
		if (!component.m_DataModelId.IsEmpty())
		{
			std::string modelName;
			gui::RmlGlobal::GetDataModelFactory().GetModelName(component.m_DataModelId, modelName);
			guiExt.DestroyDataModel(component.m_Id, modelName);
		}

		guiExt.DestroyContext(component.m_Id);
	}
}

//-------------------------------------
// GuiCanvasComponent::OnScenePostLoad
//
void GuiCanvasComponent::OnScenePostLoad(EcsController& ecs, T_EntityId const id)
{
	ET_UNUSED(id);
	if (m_RenderMode == E_RenderMode::WorldSpace)
	{
		UpdateCamera();
	}
	else if ((m_RenderMode == E_RenderMode::ScreenSpaceOverlay) && (m_Id == core::INVALID_SLOT_ID))
	{
		InitForScreenSpace(ecs);
	}
}

//---------------------------------
// GuiCanvasComponent::GetDocument
//
Rml::ElementDocument* GuiCanvasComponent::GetDocument() 
{
	if (m_Id == gui::INVALID_CONTEXT_ID)
	{
		return nullptr;
	}

	return UnifiedScene::Instance().GetGuiExtension()->GetDocument(m_Id);
}

//---------------------------------
// GuiCanvasComponent::GetDocument
//
Rml::ElementDocument const* GuiCanvasComponent::GetDocument() const
{
	if (m_Id == gui::INVALID_CONTEXT_ID)
	{
		return nullptr;
	}

	return UnifiedScene::Instance().GetGuiExtension()->GetDocument(m_Id);
}

//---------------------------------
// GuiCanvasComponent::SetActive
//
void GuiCanvasComponent::SetActive(bool const isActive)
{
	m_IsActive = isActive;
	if (m_Id != gui::INVALID_CONTEXT_ID)
	{
		UnifiedScene::Instance().GetGuiExtension()->SetContextActive(m_Id, isActive);
	}
}

//---------------------------------
// GuiCanvasComponent::SetDocument
//
void GuiCanvasComponent::SetDocument(core::HashString const guiDocId)
{
	m_GuiDocumentId = guiDocId;
	if (m_Id != gui::INVALID_CONTEXT_ID)
	{
		UnifiedScene::Instance().GetGuiExtension()->SetLoadedDocument(m_Id, guiDocId);
	}
}


//------------------------------------
// GuiCanvasComponent::SetCamera
//
void GuiCanvasComponent::SetCamera(T_EntityId const cameraEntity)
{
	ET_ASSERT(m_RenderMode == E_RenderMode::WorldSpace, "Camera can only be changed after component creation for World Space GUI components");
	m_Camera.SetId(cameraEntity);
	if (m_Id != gui::INVALID_CONTEXT_ID)
	{
		UpdateCamera();
	}
}

//---------------------------------
// GuiCanvasComponent::SetColor
//
void GuiCanvasComponent::SetColor(vec4 const& color)
{
	m_Color = color;
	if ((m_Id != gui::INVALID_CONTEXT_ID) && (m_RenderMode == E_RenderMode::WorldSpace))
	{
		UnifiedScene::Instance().GetGuiExtension()->SetContextColor(m_Id, m_Color);
	}
}

//-----------------------------------------
// GuiCanvasComponent::SetDepthTestEnabled
//
void GuiCanvasComponent::SetDepthTestEnabled(bool const depthTest)
{
	m_EnableDepthTest = depthTest;
	if ((m_Id != gui::INVALID_CONTEXT_ID) && (m_RenderMode == E_RenderMode::WorldSpace))
	{
		UnifiedScene::Instance().GetGuiExtension()->SetDepthTestEnabled(m_Id, m_EnableDepthTest);
	}
}


//----------------------------------------
// GuiCanvasComponent::InitForScreenSpace
//
// For Screen Space versions of the component we need to wait until the camera has a viewport
//
void GuiCanvasComponent::InitForScreenSpace(EcsController const& ecs)
{
	ET_ASSERT(m_Camera.GetId() != INVALID_ENTITY_ID);
	ET_ASSERT(ecs.HasComponent<CameraComponent>(m_Camera.GetId()));
	CameraComponent const& camera = ecs.GetComponent<CameraComponent>(m_Camera.GetId());
	if (camera.GetViewport() != nullptr)
	{
		GuiExtension& guiExt = *UnifiedScene::Instance().GetGuiExtension();

		m_Id = guiExt.CreateContext(camera.GetViewport());
		if (!m_DataModelId.IsEmpty())
		{
			m_DataModel = std::move(guiExt.InstantiateDataModel(m_Id, m_DataModelId));
		}

		guiExt.SetLoadedDocument(m_Id, m_GuiDocumentId);
		guiExt.SetContextActive(m_Id, m_IsActive);
	}
}

//---------------------------------------
// GuiCanvasComponent::UpdateCamera
//
// Link up the camera once the entity link is resolved
//
void GuiCanvasComponent::UpdateCamera()
{
	UnifiedScene& uniScene = UnifiedScene::Instance();
	ET_ASSERT(m_Id != gui::INVALID_CONTEXT_ID);
	if (m_Camera.GetId() != INVALID_ENTITY_ID)
	{
		EcsController& ecs = uniScene.GetEcs();

		ET_ASSERT(ecs.HasComponent<CameraComponent>(m_Camera.GetId()));
		CameraComponent const& eventCamera = ecs.GetComponent<CameraComponent>(m_Camera.GetId());

		if (eventCamera.GetViewport() != nullptr)
		{
			uniScene.GetGuiExtension()->SetEventCamera(m_Id, eventCamera.GetId());
			return;
		}
	}

	uniScene.GetGuiExtension()->SetEventCamera(m_Id, core::INVALID_SLOT_ID);
}


} // namespace fw
} // namespace et
