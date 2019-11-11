#pragma once
#include <typeinfo>
#include <EtCore/Containers/slot_map.h>


class TransformComponent;
class DirectionalShadowData;

namespace render {


	//struct PointLight
	//{
	//	vec3 m_Color;
	//	float m_Brightness;
	//	vec3 m_Position;
	//	float m_Radius;
	//};
	//struct ShadedDirectionalLight
	//{
	//	vec3 m_Color;
	//	float m_Brightness;
	//	vec3 m_Direction;
	//	DirectionalShadowData m_ShadowData;
	//};
	struct DirectionalLight
	{
		vec3 m_Color;
		float m_Brightness;
		vec3 m_Direction;
	};

	typedef core::slot_map<DirectionalLight>::id_type T_DirLightId;


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

	virtual void AddToRenderScene(TransformComponent const* const transf) = 0;
	virtual void RemoveFromRenderScene() = 0;

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

	void AddToRenderScene(TransformComponent const* const transf) override { UNUSED(transf); }
	void RemoveFromRenderScene() override {}

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
	virtual bool IsShadowEnabled() { return m_pShadowData != nullptr; }
	virtual void GenerateShadow(TransformComponent* pTransform);

	void AddToRenderScene(TransformComponent const* const transf) override;
	void RemoveFromRenderScene() override;
protected:

	DirectionalShadowData* m_pShadowData = nullptr;
	render::T_DirLightId m_LightId;
};
