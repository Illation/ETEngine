#pragma once
#include <typeinfo>

class TransformComponent;
class DirectionalShadowData;

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

	//Shadow stuff
	virtual void SetShadowEnabled( bool enabled ) { UNUSED( enabled ); }
	virtual bool IsShadowEnabled() { return false; }
	virtual void GenerateShadow( TransformComponent* pTransform ) { UNUSED( pTransform ); }

protected:
	vec3 color;
	float brightness;
	friend class LightComponent;
	virtual void DrawVolume( TransformComponent* pTransform ) { UNUSED( pTransform ); };
	bool m_Update = true;
};

class PointLight : public Light
{
public:
	PointLight(vec3 col = vec3(1, 1, 1), float brightness = 1, float rad = 1)
		:Light(col, brightness), radius(rad){}

	void SetRadius(float rad) { radius = rad;  m_Update = true;}
	float GetRadius() { return radius; }

	void DrawVolume(TransformComponent* pTransform);

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

	void DrawVolume(TransformComponent* pTransform);

	//Shadow stuff
	virtual void SetShadowEnabled(bool enabled);
	virtual bool IsShadowEnabled() { return m_pShadowData?true:false; }
	virtual void GenerateShadow(TransformComponent* pTransform);
protected:

	DirectionalShadowData* m_pShadowData = nullptr;
};
