#include "stdafx.h"
#include "LightComponent.h"

#include <EtCore/Reflection/Registration.h>


namespace et {
namespace fw {


// reflection
//------------

RTTR_REGISTRATION
{
	rttr::registration::enumeration<LightComponent::E_Type>("light component type") (
		rttr::value("Point", LightComponent::E_Type::Point),
		rttr::value("Directional", LightComponent::E_Type::Directional));

	BEGIN_REGISTER_CLASS(LightComponent, "light component")
		.property("type", &LightComponent::m_Type)
		.property("color", &LightComponent::m_Color)
		.property("brightness", &LightComponent::m_Brightness)
		.property("casts shadow", &LightComponent::m_CastsShadow)
	END_REGISTER_CLASS_POLYMORPHIC(LightComponent, I_ComponentDescriptor);
}

ECS_REGISTER_COMPONENT(LightComponent);

DEFINE_FORCED_LINKING(LightComponentLinkEnforcer)


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


} // namespace fw
} // namespace et
