#include "stdafx.h"
#include "CelestialBodyComponent.h"

#include <EtCore/Reflection/Registration.h>


namespace et {
namespace demo {


// reflection
//------------

RTTR_REGISTRATION
{
	rttr::registration::class_<CelestialBodyComponent>("celestial body component");

	BEGIN_REGISTER_CLASS(CelestialBodyComponentDesc, "celestial body comp desc")
		.property("rotation speed", &CelestialBodyComponentDesc::rotationSpeed)
		.property("start rotating", &CelestialBodyComponentDesc::startRotating)
	END_REGISTER_CLASS_POLYMORPHIC(CelestialBodyComponentDesc, fw::I_ComponentDescriptor);
}
DEFINE_FORCED_LINKING(CelestialBodyComponentDesc) // force the linker to include this unit

ECS_REGISTER_COMPONENT(CelestialBodyComponent);


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
} // namespace et
