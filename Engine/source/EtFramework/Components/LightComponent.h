#pragma once
#include <EtCore/Containers/slot_map.h>


namespace et {
	REGISTRATION_NS(fw);
}


namespace et {
namespace fw {


//---------------------------------
// LightComponent
//
// Component that lights the scene
//
class LightComponent final : public SimpleComponentDescriptor
{
	// definitions
	//-------------
	ECS_DECLARE_COMPONENT

	RTTR_ENABLE(SimpleComponentDescriptor) // for serialization
	REGISTRATION_FRIEND_NS(fw)

	friend class LightSystem;

public:
	enum class E_Type
	{
		Point,
		Directional,

		Invalid
	};

	// construct destruct
	//--------------------
private:
	LightComponent() = default;
public:
	LightComponent(E_Type const type, vec3 const& color = vec3(1.f), float const brightness = 1.f, bool const castsShadow = false);
	~LightComponent() = default;

	// accessors
	//-----------
public:
	core::T_SlotId GetLightId() const { return m_LightId; }

	vec3 const& GetColor() const { return m_Color; }
	float GetBrightness() const { return m_Brightness; }

	// functionality
	//---------------
	void SetColor(vec3 const& col) { m_Color = col; m_ColorChanged = true; }
	void SetBrightness(float const value) { m_Brightness = value; m_ColorChanged = true; }

private:

	// Data
	///////

	E_Type m_Type = LightComponent::E_Type::Invalid;
	vec3 m_Color;
	float m_Brightness = 1.f;
	bool m_CastsShadow = false;

	bool m_ColorChanged = false;

	core::T_SlotId m_LightId = core::INVALID_SLOT_ID;
};


} // namespace fw
} // namespace et
