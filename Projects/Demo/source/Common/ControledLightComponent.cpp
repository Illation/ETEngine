#include "stdafx.h"
#include "ControledLightComponent.h"

#include <EtCore/Reflection/Registration.h>


namespace et {
namespace demo {


// reflection
//------------

RTTR_REGISTRATION
{
	rttr::registration::class_<ControledLightComponent>("controled light component");

	BEGIN_REGISTER_CLASS(ControledLightComponentDesc, "controled light comp desc")
	END_REGISTER_CLASS_POLYMORPHIC(ControledLightComponentDesc, fw::I_ComponentDescriptor);
}
DEFINE_FORCED_LINKING(ControledLightComponentDesc) // force the linker to include this unit

ECS_REGISTER_COMPONENT(ControledLightComponent);


//======================================
// Controled Light Component Descriptor
//======================================


//---------------------------------------
// ControledLightComponentDesc::MakeData
//
// Create a spawn component from a descriptor
//
ControledLightComponent* ControledLightComponentDesc::MakeData()
{
	return new ControledLightComponent();
}


} // namespace demo
} // namespace et
