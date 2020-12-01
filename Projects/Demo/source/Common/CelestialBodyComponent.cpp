#include "stdafx.h"
#include "CelestialBodyComponent.h"

#include <EtCore/Reflection/Registration.h>


namespace et {
namespace demo {


// reflection
//------------

RTTR_REGISTRATION
{
	BEGIN_REGISTER_CLASS(CelestialBodyComponent, "celestial body component")
		.property("rotation speed", &CelestialBodyComponent::rotationSpeed)
		.property("start rotating", &CelestialBodyComponent::isRotating)
	END_REGISTER_CLASS_POLYMORPHIC(CelestialBodyComponent, fw::I_ComponentDescriptor);
}
DEFINE_FORCED_LINKING(CelestialBodyComponent) // force the linker to include this unit

ECS_REGISTER_COMPONENT(CelestialBodyComponent);


} // namespace demo
} // namespace et
