#include "stdafx.h"
#include "SpriteComponent.h"

#include <EtFramework/SceneGraph/SceneManager.h>

#include <EtFramework/Components/TransformComponent.h>
#include <EtFramework/SceneGraph/UnifiedScene.h>


// deprecated
//------------


SpriteComponent::SpriteComponent(T_Hash const textureAsset, vec2 const& pivot, vec4 const& color)
	: m_TextureAssetId(textureAsset)
	, m_Pivot(pivot)
	, m_Color(color)
{}

void SpriteComponent::Init()
{
	m_Id = SceneManager::GetInstance()->GetRenderScene().AddSprite(m_TextureAssetId, GetTransform()->GetNodeId(), m_Pivot, m_Color);
}

void SpriteComponent::Deinit()
{
	if (m_Id != core::INVALID_SLOT_ID)
	{
		SceneManager::GetInstance()->GetRenderScene().RemoveSprite(m_Id);
	}
}

void SpriteComponent::SetTexture(T_Hash const textureAsset)
{
	m_TextureAssetId = textureAsset;
	if (m_Id != core::INVALID_SLOT_ID)
	{
		SceneManager::GetInstance()->GetRenderScene().UpdateSpriteTexture(m_Id, m_TextureAssetId);
	}
}

void SpriteComponent::SetPivot(vec2 const& pivot)
{
	m_Pivot = pivot;
	if (m_Id != core::INVALID_SLOT_ID)
	{
		SceneManager::GetInstance()->GetRenderScene().UpdateSpritePivot(m_Id, m_Pivot);
	}
}

void SpriteComponent::SetColor(vec4 const& color)
{
	m_Color = color;
	if (m_Id != core::INVALID_SLOT_ID)
	{
		SceneManager::GetInstance()->GetRenderScene().UpdateSpriteColor(m_Id, m_Color);
	}
}


// reflection
//------------

RTTR_REGISTRATION
{
	using namespace rttr;

	registration::class_<fw::SpriteComponent>("sprite component");

	registration::class_<fw::SpriteComponentDesc>("sprite comp desc")
		.constructor<fw::SpriteComponentDesc const&>()
		.constructor<>()(rttr::detail::as_object())
		.property("texture asset", &fw::SpriteComponentDesc::textureAsset)
		.property("pivot", &fw::SpriteComponentDesc::pivot)
		.property("color", &fw::SpriteComponentDesc::color);

	rttr::type::register_converter_func([](fw::SpriteComponentDesc& descriptor, bool& ok) -> fw::I_ComponentDescriptor*
	{
		ok = true;
		return new fw::SpriteComponentDesc(descriptor);
	});
}

// component registration
//------------------------

ECS_REGISTER_COMPONENT(fw::SpriteComponent);


namespace fw {


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

	component.m_Id = SceneManager::GetInstance()->GetRenderScene().AddSprite(component.m_TextureAssetId, 
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
		SceneManager::GetInstance()->GetRenderScene().RemoveSprite(component.m_Id);
	}
}

//------------------------
// SpriteComponent::c-tor
//
SpriteComponent::SpriteComponent(T_Hash const textureAsset, vec2 const& pivot, vec4 const& color)
	: m_TextureAssetId(textureAsset)
	, m_Pivot(pivot)
	, m_Color(color)
{}

//-----------------------------
// SpriteComponent::SetTexture
//
void SpriteComponent::SetTexture(T_Hash const textureAsset)
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
	return new SpriteComponent(GetHash(textureAsset), pivot, color);
}


} // namespace fw
