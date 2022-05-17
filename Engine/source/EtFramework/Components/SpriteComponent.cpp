#include "stdafx.h"
#include "SpriteComponent.h"

#include <EtCore/Reflection/Registration.h>

#include <EtGUI/GuiExtension.h>

#include <EtFramework/Components/TransformComponent.h>
#include <EtFramework/SceneGraph/UnifiedScene.h>


namespace et {
namespace fw {


// reflection
//------------

RTTR_REGISTRATION
{
	BEGIN_REGISTER_CLASS(SpriteComponent, "sprite component")
		.property("texture asset", &SpriteComponent::m_TextureAssetId)
		.property("pivot", &SpriteComponent::m_Pivot)
		.property("color", &SpriteComponent::m_Color)
	END_REGISTER_CLASS_POLYMORPHIC(SpriteComponent, I_ComponentDescriptor);
}

ECS_REGISTER_COMPONENT(SpriteComponent);


//==================
// Sprite Component 
//==================


//-----------------------------------
// SpriteComponent::OnComponentAdded
//
// Register sprites in the render scene when they are added to the ECS
//
void SpriteComponent::OnComponentAdded(EcsController& controller, SpriteComponent& component, T_EntityId const entity)
{
	ET_ASSERT(component.m_Id == core::INVALID_SLOT_ID);

	component.m_Id = UnifiedScene::Instance().GetGuiExtension()->AddSprite(component.m_TextureAssetId,
		controller.GetComponent<TransformComponent>(entity).GetNodeId(), 
		component.m_Pivot,
		component.m_Color);
}

//-----------------------------------
// SpriteComponent::OnComponentAdded
//
// Unregister sprites in the render scene when they are added to the ECS
//
void SpriteComponent::OnComponentRemoved(EcsController& controller, SpriteComponent& component, T_EntityId const entity)
{
	UNUSED(controller);
	UNUSED(entity);

	if (component.m_Id != core::INVALID_SLOT_ID)
	{
		UnifiedScene::Instance().GetGuiExtension()->RemoveSprite(component.m_Id);
	}
}

//------------------------
// SpriteComponent::c-tor
//
SpriteComponent::SpriteComponent(core::HashString const textureAsset, vec2 const& pivot, vec4 const& color)
	: m_TextureAssetId(textureAsset)
	, m_Pivot(pivot)
	, m_Color(color)
{}

//-----------------------------
// SpriteComponent::SetTexture
//
void SpriteComponent::SetTexture(core::HashString const textureAsset)
{
	m_TextureAssetId = textureAsset;
	if (m_Id != core::INVALID_SLOT_ID)
	{
		UnifiedScene::Instance().GetGuiExtension()->UpdateSpriteTexture(m_Id, m_TextureAssetId);
	}
}

//---------------------------
// SpriteComponent::SetPivot
//
void SpriteComponent::SetPivot(vec2 const& pivot)
{
	m_Pivot = pivot;
	if (m_Id != core::INVALID_SLOT_ID)
	{
		UnifiedScene::Instance().GetGuiExtension()->UpdateSpritePivot(m_Id, m_Pivot);
	}
}

//---------------------------
// SpriteComponent::SetColor
//
void SpriteComponent::SetColor(vec4 const& color)
{
	m_Color = color;
	if (m_Id != core::INVALID_SLOT_ID)
	{
		UnifiedScene::Instance().GetGuiExtension()->UpdateSpriteColor(m_Id, m_Color);
	}
}


} // namespace fw
} // namespace et
