#include "stdafx.h"
#include "ControledLightComponent.h"

#include <EtCore/Reflection/Registration.h>


namespace et {
namespace demo {


// reflection
//------------

RTTR_REGISTRATION
{
	BEGIN_REGISTER_CLASS(ControledLightComponent, "controled light component")
	END_REGISTER_CLASS_POLYMORPHIC(ControledLightComponent, fw::I_ComponentDescriptor);
}
DEFINE_FORCED_LINKING(ControledLightComponent) // force the linker to include this unit

ECS_REGISTER_COMPONENT(ControledLightComponent);


} // namespace demo
} // namespace et
