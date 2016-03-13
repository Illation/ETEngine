#pragma once
#include "../SceneGraph/Entity.hpp"

class CameraComponent;

class FixedCamera : public Entity
{
public:
	FixedCamera();
	virtual ~FixedCamera();

protected:

	virtual void Initialize();

private:

	CameraComponent *m_pCamera = nullptr;

private:
	// -------------------------
	// Disabling default copy constructor and default 
	// assignment operator.
	// -------------------------
	FixedCamera(const FixedCamera& yRef);
	FixedCamera& operator=(const FixedCamera& yRef);
};

