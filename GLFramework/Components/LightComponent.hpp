#pragma once
#include "AbstractComponent.hpp"
#include <typeinfo>

class Light
{
public:
	Light(glm::vec3 col = glm::vec3(1, 1, 1))
		:color(col) {}

	void SetColor(glm::vec3 col) { color = col; m_Update = true; }
	glm::vec3 GetColor() { return color; }

protected:
	glm::vec3 color;
	friend class LightComponent;
	virtual void UploadVariables(GLuint program, glm::vec3 pos, unsigned index) = 0;
	bool m_Update = true;
};
class PointLight : public Light
{
public:
	PointLight(glm::vec3 col = glm::vec3(1, 1, 1),
		float lin = 1, float quad = 1)
		:Light(col), linear(lin), quadratic(quad){}

	void SetLinear(float lin) { linear = lin;  m_Update = true;}
	float GetLinear() { return linear; }
	void SetQuadratic(float quad) { quadratic = quad;  m_Update = true; }
	float GetQuadratic() { return quadratic; }

protected:
	float linear;
	float quadratic;
	virtual void UploadVariables(GLuint program, glm::vec3 pos, unsigned index);
};
class DirectionalLight : public Light
{
public:
	DirectionalLight(glm::vec3 col = glm::vec3(1, 1, 1),
		glm::vec3 dir = glm::vec3(0, -1, 0))
		:Light(col), direction(dir){}

	void SetDirection(glm::vec3 dir) { direction = dir; m_Update = true; }
	glm::vec3 GetDirection() { return direction; }

protected:
	glm::vec3 direction;
	virtual void UploadVariables(GLuint program, glm::vec3 pos, unsigned index);
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

