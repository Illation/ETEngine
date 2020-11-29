#include "stdafx.h"
#include "PlanetComponent.h"

#include <EtCore/Reflection/Registration.h>


namespace et {
namespace fw {


// reflection
//------------

RTTR_REGISTRATION
{
	rttr::registration::class_<PlanetComponent>("planet component");
	
	BEGIN_REGISTER_CLASS(PlanetComponentDesc, "planet comp desc")
		.property("radius", &PlanetComponentDesc::radius)
		.property("height", &PlanetComponentDesc::height)
		.property("diffuse texture", &PlanetComponentDesc::texDiffuse)
		.property("detail texture 1", &PlanetComponentDesc::texDetail1)
		.property("detail texture 2", &PlanetComponentDesc::texDetail2)
		.property("height texture", &PlanetComponentDesc::texHeight)
		.property("height detail texture", &PlanetComponentDesc::texHeightDetail)
	END_REGISTER_CLASS_POLYMORPHIC(PlanetComponentDesc, I_ComponentDescriptor);

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


//=============================
// Planet Component Descriptor
//=============================


//-------------------------------
// PlanetComponentDesc::MakeData
//
// Create a planet component from a descriptor
//
PlanetComponent* PlanetComponentDesc::MakeData()
{
	render::PlanetParams params;

	params.radius = radius;
	params.height = height;

	params.texDiffuseId = GetHash(texDiffuse);
	params.texDetail1Id = GetHash(texDetail1);
	params.texDetail2Id = GetHash(texDetail2);

	params.texHeightId = GetHash(texHeight);
	params.texHeightDetailId = GetHash(texHeightDetail);

	return new PlanetComponent(params);
}


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
