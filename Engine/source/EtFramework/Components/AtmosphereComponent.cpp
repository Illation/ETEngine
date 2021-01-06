#include "stdafx.h"
#include "AtmosphereComponent.h"

#include "TransformComponent.h"
#include "LightComponent.h"

#include <EtCore/Reflection/Registration.h>

#include <EtFramework/SceneGraph/UnifiedScene.h>
#include <EtFramework/ECS/EcsController.h>


namespace et {
namespace fw {


// reflection
//------------

RTTR_REGISTRATION
{
	rttr::registration::class_<AtmosphereComponent>("atmosphere component");

	BEGIN_REGISTER_CLASS(AtmosphereComponentDesc, "atmosphere comp desc")
		.property("asset", &AtmosphereComponentDesc::asset)
		.property("height", &AtmosphereComponentDesc::height)
		.property("ground radius", &AtmosphereComponentDesc::groundRadius)
		.property("sun", &AtmosphereComponentDesc::sun)
	END_REGISTER_CLASS_POLYMORPHIC(AtmosphereComponentDesc, I_ComponentDescriptor);
}

ECS_REGISTER_COMPONENT(AtmosphereComponent);

DEFINE_FORCED_LINKING(AtmosphereComponentDesc)


//======================
// Atmosphere Component 
//======================


//---------------------------------
// AtmosphereComponent::c-tor
//
AtmosphereComponent::AtmosphereComponent(core::HashString const assetId, float const height, float const groundHeight)
	: m_AssetId(assetId)
	, m_Height(height)
	, m_GroundHeight(groundHeight)
{ }

//----------------------------------
// AtmosphereComponent::SetSunlight
//
void AtmosphereComponent::SetSunlight(core::T_SlotId const lightId)
{
	if (m_RenderId != core::INVALID_SLOT_ID)
	{
		UnifiedScene::Instance().GetRenderScene().UpdateAtmosphereLight(m_RenderId, lightId);
	}
}


//=================================
// Atmosphere Component Descriptor
//=================================


//-----------------------------------
// AtmosphereComponentDesc::MakeData
//
// Create a atmosphere component from a descriptor
//
AtmosphereComponent* AtmosphereComponentDesc::MakeData()
{
	return new AtmosphereComponent(asset, height, groundRadius);
}

//------------------------------------------
// AtmosphereComponentDesc::OnScenePostLoad
//
void AtmosphereComponentDesc::OnScenePostLoad(EcsController& ecs, T_EntityId const id, AtmosphereComponent& comp)
{
	ET_ASSERT(ecs.HasComponent<LightComponent>(sun.GetId()));
	LightComponent const& light = ecs.GetComponent<LightComponent>(sun.GetId());

	comp.SetSunlight(light.GetLightId());
}


} // namespace fw
} // namespace et
