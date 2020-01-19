#include "stdafx.h"
#include "LightComponent.h"

#include <EtCore/Reflection/Registration.h>


namespace fw {


// reflection
//------------

RTTR_REGISTRATION
{
	rttr::registration::enumeration<LightComponent::E_Type>("light component type") (
		rttr::value("Point", LightComponent::E_Type::Point),
		rttr::value("Directional", LightComponent::E_Type::Directional));

	rttr::registration::class_<LightComponent>("light component");

	BEGIN_REGISTER_POLYMORPHIC_CLASS(LightComponentDesc, "light comp desc")
		.property("type", &LightComponentDesc::type)
		.property("color", &LightComponentDesc::color)
		.property("brightness", &LightComponentDesc::brightness)
		.property("casts shadow", &LightComponentDesc::castsShadow)
	END_REGISTER_POLYMORPHIC_CLASS(LightComponentDesc, I_ComponentDescriptor);
}

ECS_REGISTER_COMPONENT(LightComponent);


//=================
// Light Component 
//=================


//---------------------------------
// LightComponent::c-tor
//
LightComponent::LightComponent(E_Type const type, vec3 const& color, float const brightness, bool const castsShadow)
	: m_Type(type)
	, m_Color(color)
	, m_Brightness(brightness)
	, m_CastsShadow(castsShadow)
{
	ET_ASSERT((!castsShadow) || (type == E_Type::Directional), "Point light shadows are not supported");
}


//============================
// Light Component Descriptor
//============================


//------------------------------
// LightComponentDesc::MakeData
//
// Create a light component from a descriptor
//
LightComponent* LightComponentDesc::MakeData()
{
	return new LightComponent(type, color, brightness, castsShadow);
}


} // namespace fw
