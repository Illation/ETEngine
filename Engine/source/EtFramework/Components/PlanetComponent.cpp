#include "stdafx.h"
#include "PlanetComponent.h"

#include <rttr/registration>


// reflection
//------------

RTTR_REGISTRATION
{
	using namespace rttr;

	registration::class_<fw::PlanetComponent>("planet component");

	registration::class_<fw::PlanetCameraLinkComponent>("planet camera link component");

	registration::class_<fw::PlanetComponentDesc>("planet comp desc")
		.constructor<fw::PlanetComponentDesc const&>()
		.constructor<>()(rttr::detail::as_object())
		.property("radius", &fw::PlanetComponentDesc::radius)
		.property("height", &fw::PlanetComponentDesc::height)
		.property("diffuse texture", &fw::PlanetComponentDesc::texDiffuse)
		.property("detail texture 1", &fw::PlanetComponentDesc::texDetail1)
		.property("detail texture 2", &fw::PlanetComponentDesc::texDetail2)
		.property("height texture", &fw::PlanetComponentDesc::texHeight)
		.property("height detail texture", &fw::PlanetComponentDesc::texHeightDetail);

	rttr::type::register_converter_func([](fw::PlanetComponentDesc& descriptor, bool& ok) -> fw::I_ComponentDescriptor*
	{
		ok = true;
		return new fw::PlanetComponentDesc(descriptor);
	});

	registration::class_<fw::PlanetCameraLinkComponentDesc>("planet camera link comp desc")
		.constructor<fw::PlanetCameraLinkComponentDesc const&>()
		.constructor<>()(rttr::detail::as_object())
		.property("planet", &fw::PlanetCameraLinkComponentDesc::planet);

	rttr::type::register_converter_func([](fw::PlanetCameraLinkComponentDesc& descriptor, bool& ok) -> fw::I_ComponentDescriptor*
	{
		ok = true;
		return new fw::PlanetCameraLinkComponentDesc(descriptor);
	});
}

// component registration
//------------------------

ECS_REGISTER_COMPONENT(fw::PlanetComponent);
ECS_REGISTER_COMPONENT(fw::PlanetCameraLinkComponent);


namespace fw {


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
