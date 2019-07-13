#pragma once
#include "AbstractComponent.h"

#include <typeinfo>


class TransformComponent;
class Light;

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
	T* GetLight()
	{
		return dynamic_cast<T*>(m_Light);
	}

	void UploadVariables(GLuint shaderProgram, uint32 index);

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

