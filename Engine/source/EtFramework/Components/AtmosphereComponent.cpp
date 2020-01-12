#include "stdafx.h"
#include "AtmosphereComponent.h"

#include <rttr/registration>

#include "TransformComponent.h"
#include "LightComponent.h"

#include <EtFramework/SceneGraph/UnifiedScene.h>
#include <EtFramework/ECS/EcsController.h>


// reflection
//------------

RTTR_REGISTRATION
{
	using namespace rttr;

	registration::class_<fw::AtmosphereComponent>("atmosphere component");

	registration::class_<fw::AtmosphereComponentDesc>("atmosphere comp desc")
		.constructor<fw::AtmosphereComponentDesc const&>()
		.constructor<>()(rttr::detail::as_object())
		.property("asset", &fw::AtmosphereComponentDesc::asset)
		.property("height", &fw::AtmosphereComponentDesc::height)
		.property("ground radius", &fw::AtmosphereComponentDesc::groundRadius)
		.property("sun", &fw::AtmosphereComponentDesc::sun);

	rttr::type::register_converter_func([](fw::AtmosphereComponentDesc& descriptor, bool& ok) -> fw::I_ComponentDescriptor*
	{
		ok = true;
		return new fw::AtmosphereComponentDesc(descriptor);
	});
}

// component registration
//------------------------

ECS_REGISTER_COMPONENT(fw::AtmosphereComponent);


namespace fw {


//======================
// Atmosphere Component 
//======================


//---------------------------------
// AtmosphereComponent::c-tor
//
AtmosphereComponent::AtmosphereComponent(T_Hash const assetId, float const height, float const groundHeight)
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
	return new AtmosphereComponent(GetHash(asset), height, groundRadius);
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
