#pragma once
#include "AbstractComponent.h"
#include <EtRendering/GraphicsTypes/Light.h>

#include <typeinfo>


class TransformComponent;


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

	render::T_LightId GetLightId() const { return m_LightId; }

protected:

	Light* m_Light = nullptr;

	virtual void Initialize();
	virtual void Update();
	virtual void Draw() {}
	virtual void DrawForward() {}

private:
	friend class TransformComponent;
	
	bool m_PositionUpdated = false;
	render::T_LightId m_LightId = core::INVALID_SLOT_ID;

private:
	// -------------------------
	// Disabling default copy constructor and default 
	// assignment operator.
	// -------------------------
	LightComponent(const LightComponent& yRef);
	LightComponent& operator=(const LightComponent& yRef);
};

