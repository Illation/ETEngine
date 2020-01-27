#include "stdafx.h"
#include "SpriteComponent.h"

#include <EtCore/Reflection/Registration.h>

#include <EtFramework/Components/TransformComponent.h>
#include <EtFramework/SceneGraph/UnifiedScene.h>


namespace et {
namespace fw {


// reflection
//------------

RTTR_REGISTRATION
{
	rttr::registration::class_<SpriteComponent>("sprite component");

	BEGIN_REGISTER_POLYMORPHIC_CLASS(SpriteComponentDesc, "sprite comp desc")
		.property("texture asset", &SpriteComponentDesc::textureAsset)
		.property("pivot", &SpriteComponentDesc::pivot)
		.property("color", &SpriteComponentDesc::color)
	END_REGISTER_POLYMORPHIC_CLASS(SpriteComponentDesc, I_ComponentDescriptor);
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

	component.m_Id = UnifiedScene::Instance().GetRenderScene().AddSprite(component.m_TextureAssetId, 
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
		UnifiedScene::Instance().GetRenderScene().RemoveSprite(component.m_Id);
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
		UnifiedScene::Instance().GetRenderScene().UpdateSpriteTexture(m_Id, m_TextureAssetId);
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
		UnifiedScene::Instance().GetRenderScene().UpdateSpritePivot(m_Id, m_Pivot);
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
		UnifiedScene::Instance().GetRenderScene().UpdateSpriteColor(m_Id, m_Color);
	}
}


//=============================
// Sprite Component Descriptor
//=============================


//-------------------------------
// SpriteComponentDesc::MakeData
//
// Create a sprite component from a descriptor
//
SpriteComponent* SpriteComponentDesc::MakeData()
{
	return new SpriteComponent(textureAsset, pivot, color);
}


} // namespace fw
} // namespace et
