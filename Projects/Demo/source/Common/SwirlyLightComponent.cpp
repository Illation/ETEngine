#include "stdafx.h"
#include "SwirlyLightComponent.h"

#include <EtCore/Reflection/Registration.h>
#include <EtCore/Util/GlobalRandom.h>

#include <EtFramework/ECS/EcsController.h>
#include <EtFramework/Components/TransformComponent.h>


namespace et {
namespace demo {


// reflection
//------------

RTTR_REGISTRATION
{
	rttr::registration::class_<SwirlyLightComponent>("swirly light component");

	BEGIN_REGISTER_POLYMORPHIC_CLASS(SwirlyLightComponentDesc, "swirly light comp desc")
		.property("min radius", &SwirlyLightComponentDesc::minRadius)
		.property("max radius", &SwirlyLightComponentDesc::maxRadius)
		.property("min time mult", &SwirlyLightComponentDesc::minTimeMult)
		.property("max time mult", &SwirlyLightComponentDesc::maxTimeMult)
		.property("bounds", &SwirlyLightComponentDesc::bounds)
	END_REGISTER_POLYMORPHIC_CLASS(SwirlyLightComponentDesc, fw::I_ComponentDescriptor);
}
DEFINE_FORCED_LINKING(SwirlyLightComponentDesc) // force the linker to include this unit

ECS_REGISTER_COMPONENT(SwirlyLightComponent);


//===================================
// Swirly Light Component Descriptor
//===================================


//------------------------------------
// SwirlyLightComponentDesc::MakeData
//
// Create a spawn component from a descriptor
//
SwirlyLightComponent* SwirlyLightComponentDesc::MakeData()
{
	etm::Random& random = core::GetGlobalRandom();

	SwirlyLightComponent* const ret = new SwirlyLightComponent();

	ret->radius1 = random.RandFloat(minRadius, maxRadius);
	ret->radius2 = random.RandFloat(minRadius, maxRadius);

	ret->angle1 = random.RandFloat(0.f, etm::PI2);
	ret->angle2 = random.RandFloat(0.f, etm::PI2);

	ret->timeOffset1 = random.RandFloat(0.f, etm::PI2);
	ret->timeOffset2 = random.RandFloat(0.f, etm::PI2);

	ret->timeMult1 = random.RandFloat(minTimeMult, maxTimeMult);
	ret->timeMult2 = random.RandFloat(maxTimeMult, maxTimeMult);

	return ret;
}

//-------------------------------------------
// SwirlyLightComponentDesc::OnScenePostLoad
//
// Offset relative to transform, set origin
//
void SwirlyLightComponentDesc::OnScenePostLoad(fw::EcsController& ecs, fw::T_EntityId const id, SwirlyLightComponent& comp)
{
	etm::Random& random = core::GetGlobalRandom();

	ET_ASSERT(ecs.HasComponent<fw::TransformComponent>(id));
	fw::TransformComponent& transform = ecs.GetComponent<fw::TransformComponent>(id);

	comp.origin = transform.GetPosition();

	transform.Translate(random.RandFloat(-bounds, bounds), random.RandFloat(-bounds, bounds), random.RandFloat(-bounds, bounds));
}


} // namespace demo
} // namespace et
