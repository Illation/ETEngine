#pragma once
#include <typeinfo>
#include <EtCore/Containers/slot_map.h>


class DirectionalShadowData;


namespace render {


//------------------
// Light
//
// All data required to render a light
//
struct Light
{
	vec3 m_Color;
	core::T_SlotId m_NodeId; // extract forward vector from transform for directional lights, or size and position for point lights
};

typedef core::T_SlotId T_LightId;


} // namespace render

class Light
{
public:
	Light(vec3 col = vec3(1, 1, 1)
		, float b = 1)
		:color(col), brightness(b) {}
	virtual ~Light() {}

	//Basic light
	void SetColor(vec3 col) { color = col; m_Update = true; }
	vec3 GetColor() { return color; }
	void SetBrightness(float b) { brightness = b; m_Update = true; }
	float GetBrightness() { return brightness; }

	virtual bool IsDirectional() const { return false; }

	//Shadow stuff
	virtual void SetShadowEnabled( bool enabled ) { UNUSED( enabled ); }
	virtual bool IsShadowEnabled() { return false; }

protected:
	vec3 color;
	float brightness;
	bool m_Update = true;
};

class PointLight : public Light
{
public:
	PointLight(vec3 col = vec3(1, 1, 1), float brightness = 1, float rad = 1)
		:Light(col, brightness), radius(rad){}

	void SetRadius(float rad) { radius = rad;  m_Update = true;}
	float GetRadius() { return radius; }

protected:
	float radius;
};

class DirectionalLight : public Light
{
public:
	DirectionalLight(vec3 col = vec3(1, 1, 1), float brightness = 1)
		:Light(col, brightness){}
	virtual ~DirectionalLight()
	{
		SetShadowEnabled(false);
	}

	bool IsDirectional() const override { return true; }

	//Shadow stuff
	virtual void SetShadowEnabled(bool enabled);
	virtual bool IsShadowEnabled() { return m_pShadowData != nullptr; }

protected:

	DirectionalShadowData* m_pShadowData = nullptr;
};
