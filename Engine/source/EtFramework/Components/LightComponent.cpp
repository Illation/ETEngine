#include "stdafx.h"
#include "LightComponent.h"

#include <EtFramework/SceneGraph/Entity.h>
#include <EtFramework/SceneGraph/SceneManager.h>


// deprecated
//------------


//=================
// Light Component
//=================


//---------------------------------
// LightComponent::c-tor
//
LightComponent::LightComponent(Type const type, vec3 const& color, float const brightness, bool const castsShadow)
	: m_Type(type)
	, m_Color(color)
	, m_Brightness(brightness)
	, m_CastsShadow(castsShadow)
{
	ET_ASSERT((!castsShadow) || (type == Type::Directional), "Point light shadows are not supported");
}

//---------------------------------
// LightComponent::Init
//
void LightComponent::Init()
{
	vec3 const col = m_Color * m_Brightness;

	m_LightId = SceneManager::GetInstance()->GetRenderScene().AddLight(col, 
		GetTransform()->GetNodeId(), 
		m_Type == Type::Directional,
		m_CastsShadow);
}

//---------------------------------
// LightComponent::Deinit
//
void LightComponent::Deinit()
{
	SceneManager::GetInstance()->GetRenderScene().RemoveLight(m_LightId);
}

//---------------------------------
// LightComponent::Update
//
void LightComponent::Update()
{
	if (m_ColorChanged)
	{
		vec3 const col = m_Color * m_Brightness;
		SceneManager::GetInstance()->GetRenderScene().UpdateLightColor(m_LightId, col);

		m_ColorChanged = false;
	}
}


// reflection
//------------

RTTR_REGISTRATION
{
	using namespace rttr;

	registration::enumeration<fw::LightComponent::E_Type>("light component type") (
		value("Point", fw::LightComponent::E_Type::Point),
		value("Directional", fw::LightComponent::E_Type::Directional));

	registration::class_<fw::LightComponent>("light component");

	registration::class_<fw::LightComponentDesc>("light comp desc")
		.constructor<fw::LightComponentDesc const&>()
		.constructor<>()(rttr::detail::as_object())
		.property("type", &fw::LightComponentDesc::type)
		.property("color", &fw::LightComponentDesc::color)
		.property("brightness", &fw::LightComponentDesc::brightness)
		.property("casts shadow", &fw::LightComponentDesc::castsShadow);

	rttr::type::register_converter_func([](fw::LightComponentDesc& descriptor, bool& ok) -> fw::I_ComponentDescriptor*
	{
		ok = true;
		return new fw::LightComponentDesc(descriptor);
	});
}

// component registration
//------------------------

ECS_REGISTER_COMPONENT(fw::LightComponent);


namespace fw {


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
