#pragma once
#include "AbstractComponent.hpp"
#include <typeinfo>

class TransformComponent;
class DirectionalShadowData;

class Light
{
public:
	Light(glm::vec3 col = glm::vec3(1, 1, 1)
		, float b = 1)
		:color(col), brightness(b) {}

	//Basic light
	void SetColor(glm::vec3 col) { color = col; m_Update = true; }
	glm::vec3 GetColor() { return color; }
	void SetBrightness(float b) { brightness = b; m_Update = true; }
	float GetBrightness() { return brightness; }

	//Shadow stuff
	virtual void SetShadowEnabled(bool enabled) {}
	virtual bool IsShadowEnabled() { return false; }
	virtual void GenerateShadow(TransformComponent* pTransform) {}

protected:
	glm::vec3 color;
	float brightness;
	friend class LightComponent;
	virtual void UploadVariables(GLuint program, TransformComponent* comp, unsigned index) = 0;
	virtual void DrawVolume(TransformComponent* pTransform) {};
	bool m_Update = true;
};
class PointLight : public Light
{
public:
	PointLight(glm::vec3 col = glm::vec3(1, 1, 1),
		float brightness = 1, float rad = 1)
		:Light(col, brightness), radius(rad){}

	void SetRadius(float rad) { radius = rad;  m_Update = true;}
	float GetRadius() { return radius; }

	void DrawVolume(TransformComponent* pTransform);

protected:
	float radius;
	virtual void UploadVariables(GLuint program, TransformComponent* comp, unsigned index);
};
class DirectionalLight : public Light
{
public:
	DirectionalLight(glm::vec3 col = glm::vec3(1, 1, 1)
		, float brightness = 1)
		:Light(col, brightness){}

	void DrawVolume(TransformComponent* pTransform);

	//Shadow stuff
	virtual void SetShadowEnabled(bool enabled);
	virtual bool IsShadowEnabled() { return m_pShadowData?true:false; }
	virtual void GenerateShadow(TransformComponent* pTransform);
protected:
	virtual void UploadVariables(GLuint program, TransformComponent* comp, unsigned index);

	DirectionalShadowData* m_pShadowData;
};

class LightComponent : public AbstractComponent
{
public:
	LightComponent(Light* light);
	~LightComponent();

	template<class T>
	bool IsLightType()
	{
		return GetLight<T>() != nullptr;
	}
	template<class T>
	T* GetLight(bool searchChildren = false)
	{
		return dynamic_cast<T*>(m_Light);
	}

	glm::vec3 GetColor() { return m_Light->color; }
	void UploadVariables(GLuint shaderProgram, unsigned index);

	void DrawVolume();
	void GenerateShadow();

protected:

	Light* m_Light = nullptr;

	virtual void Initialize();
	virtual void Update();
	virtual void Draw();
	virtual void DrawForward();

private:
	friend class TransformComponent;
	
	bool m_PositionUpdated = false;

private:
	// -------------------------
	// Disabling default copy constructor and default 
	// assignment operator.
	// -------------------------
	LightComponent(const LightComponent& yRef);
	LightComponent& operator=(const LightComponent& yRef);
};

