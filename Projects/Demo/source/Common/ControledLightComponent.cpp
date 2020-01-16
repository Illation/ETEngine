#include "stdafx.h"
#include "ControledLightComponent.h"

#include <rttr/registration>


// reflection
//------------

RTTR_REGISTRATION
{
	using namespace rttr;

	registration::class_<demo::ControledLightComponent>("controled light component");

	registration::class_<demo::ControledLightComponentDesc>("controled light comp desc")
		.constructor<demo::ControledLightComponentDesc const&>()
		.constructor<>()(rttr::detail::as_object());

	rttr::type::register_converter_func([](demo::ControledLightComponentDesc& descriptor, bool& ok) -> fw::I_ComponentDescriptor*
	{
		ok = true;
		return new demo::ControledLightComponentDesc(descriptor);
	});
}

DEFINE_FORCED_LINKING(demo::ControledLightComponentDesc) // force the linker to include this unit

// component registration
//------------------------

ECS_REGISTER_COMPONENT(demo::ControledLightComponent);


namespace demo {


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
