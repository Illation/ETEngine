#pragma once
#include <EtCore/Content/AssetPointer.h>

#include <EtFramework/ECS/EcsController.h>
#include <EtFramework/SceneGraph/ComponentDescriptor.h>


namespace et {
	REGISTRATION_NS(fw);
}


namespace et {
namespace fw {


//---------------------------------
// SpriteComponent
//
// Component that causes a sprite to be drawn in the scene
//
class SpriteComponent final : public SimpleComponentDescriptor
{
	// definitions
	//-------------
	ECS_DECLARE_COMPONENT

	RTTR_ENABLE(SimpleComponentDescriptor) // for serialization
	REGISTRATION_FRIEND_NS(fw)

	friend class SpriteComponent;

	// construct destruct
	//--------------------
	SpriteComponent() = default;
public:
	SpriteComponent(core::HashString const textureAsset, vec2 const& pivot = vec2(), vec4 const& color = vec4(1.f));
	~SpriteComponent() = default;

	// init deinit
	//-------------

	static void OnComponentAdded(EcsController& controller, SpriteComponent& component, T_EntityId const entity);
	static void OnComponentRemoved(EcsController& controller, SpriteComponent& component, T_EntityId const entity);

	// accessors
	//-----------
	vec2 GetPivot() const { return m_Pivot; }
	vec4 const& GetColor() const { return m_Color; }

	// modifiers
	//-----------
	void SetPivot(vec2 const& pivot);
	void SetColor(vec4 const& color);
	void SetTexture(core::HashString const textureAsset);

	// Data
	///////

private:
	core::T_SlotId m_Id = core::INVALID_SLOT_ID;
	core::HashString m_TextureAssetId;
	vec2 m_Pivot;
	vec4 m_Color;
};


} // namespace fw
} // namespace et
