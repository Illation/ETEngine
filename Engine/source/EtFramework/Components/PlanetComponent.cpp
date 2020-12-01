#include "stdafx.h"
#include "PlanetComponent.h"

#include <EtCore/Reflection/Registration.h>


namespace et {
namespace fw {


// reflection
//------------

RTTR_REGISTRATION
{
	BEGIN_REGISTER_CLASS(PlanetComponent, "planet component")
		.property("params", &PlanetComponent::m_Params)
	END_REGISTER_CLASS_POLYMORPHIC(PlanetComponent, I_ComponentDescriptor);

	rttr::registration::class_<PlanetCameraLinkComponent>("planet camera link component");
	
	BEGIN_REGISTER_CLASS(PlanetCameraLinkComponentDesc, "planet camera link comp desc")
		.property("planet", &PlanetCameraLinkComponentDesc::planet)
	END_REGISTER_CLASS_POLYMORPHIC(PlanetCameraLinkComponentDesc, I_ComponentDescriptor);
}

ECS_REGISTER_COMPONENT(PlanetComponent);
ECS_REGISTER_COMPONENT(PlanetCameraLinkComponent);


//==================
// Planet Component 
//==================


//---------------------------------
// PlanetComponent::c-tor
//
PlanetComponent::PlanetComponent(render::PlanetParams const& params)
	: m_Params(params)
{ }


//=========================================
// Planet Camera Link Component Descriptor
//=========================================


//-------------------------------
// PlanetComponentDesc::MakeData
//
// Create a planet component from a descriptor
//
PlanetCameraLinkComponent* PlanetCameraLinkComponentDesc::MakeData()
{
	return new PlanetCameraLinkComponent(planet.id);
}


} // namespace fw
} // namespace et
