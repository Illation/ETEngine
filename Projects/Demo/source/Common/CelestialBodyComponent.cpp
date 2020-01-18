#include "stdafx.h"
#include "CelestialBodyComponent.h"

#include <rttr/registration>


// reflection
//------------

RTTR_REGISTRATION
{
	using namespace rttr;

	registration::class_<demo::CelestialBodyComponent>("celestial body component");

	registration::class_<demo::CelestialBodyComponentDesc>("celestial body comp desc")
		.constructor<demo::CelestialBodyComponentDesc const&>()
		.constructor<>()(rttr::detail::as_object())
		.property("rotation speed", &demo::CelestialBodyComponentDesc::rotationSpeed)
		.property("start rotating", &demo::CelestialBodyComponentDesc::startRotating);

	rttr::type::register_converter_func([](demo::CelestialBodyComponentDesc& descriptor, bool& ok) -> fw::I_ComponentDescriptor*
	{
		ok = true;
		return new demo::CelestialBodyComponentDesc(descriptor);
	});
}

DEFINE_FORCED_LINKING(demo::CelestialBodyComponentDesc) // force the linker to include this unit

// component registration
//------------------------

ECS_REGISTER_COMPONENT(demo::CelestialBodyComponent);


namespace demo {


//======================================
// Celestial Body Component Descriptor
//======================================


//---------------------------------------
// CelestialBodyComponentDesc::MakeData
//
// Create a spawn component from a descriptor
//
CelestialBodyComponent* CelestialBodyComponentDesc::MakeData()
{
	CelestialBodyComponent* const ret = new CelestialBodyComponent();

	ret->rotationSpeed = rotationSpeed;
	ret->isRotating = startRotating;

	return ret;
}


} // namespace demo


