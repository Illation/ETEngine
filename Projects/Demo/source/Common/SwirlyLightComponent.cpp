#include "stdafx.h"
#include "SwirlyLightComponent.h"

#include <rttr/registration>

#include <EtCore/Helper/GlobalRandom.h>

#include <EtFramework/ECS/EcsController.h>
#include <EtFramework/Components/TransformComponent.h>


// reflection
//------------

RTTR_REGISTRATION
{
	using namespace rttr;

	registration::class_<demo::SwirlyLightComponent>("swirly light component");

	registration::class_<demo::SwirlyLightComponentDesc>("swirly light comp desc")
		.constructor<demo::SwirlyLightComponentDesc const&>()
		.constructor<>()(rttr::detail::as_object())
		.property("min radius", &demo::SwirlyLightComponentDesc::minRadius)
		.property("max radius", &demo::SwirlyLightComponentDesc::maxRadius)
		.property("min time mult", &demo::SwirlyLightComponentDesc::minTimeMult)
		.property("max time mult", &demo::SwirlyLightComponentDesc::maxTimeMult)
		.property("bounds", &demo::SwirlyLightComponentDesc::bounds);

	rttr::type::register_converter_func([](demo::SwirlyLightComponentDesc& descriptor, bool& ok) -> fw::I_ComponentDescriptor*
	{
		ok = true;
		return new demo::SwirlyLightComponentDesc(descriptor);
	});
}

DEFINE_FORCED_LINKING(demo::SwirlyLightComponentDesc) // force the linker to include this unit

// component registration
//------------------------

ECS_REGISTER_COMPONENT(demo::SwirlyLightComponent);



namespace demo {


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

