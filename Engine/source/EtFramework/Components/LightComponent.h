#pragma once
#include "AbstractComponent.h"

#include <EtCore/Containers/slot_map.h>


//---------------------------------
// LightComponent
//
// Component that lights the scene
//
class LightComponent : public AbstractComponent
{
	// definitions
	//-------------
public:
	enum class Type
	{
		Point,
		Directional
	};

	// construct destruct
	//--------------------
	LightComponent(Type const type, vec3 const& color = vec3(1.f), float const brightness = 1.f, bool const castsShadow = false);
	~LightComponent() = default;

private:
	LightComponent(const LightComponent& yRef);
	LightComponent& operator=(const LightComponent& yRef);

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

	// component interface
	//---------------------
protected:
	virtual void Init() override;
	virtual void Deinit() override;
	virtual void Update() override;

private:

	// Data
	///////

	Type m_Type;
	vec3 m_Color;
	float m_Brightness;
	bool m_CastsShadow;

	bool m_ColorChanged = false;

	core::T_SlotId m_LightId = core::INVALID_SLOT_ID;
};

