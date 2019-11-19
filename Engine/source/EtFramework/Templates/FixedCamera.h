#pragma once
#include <EtFramework/SceneGraph/Entity.h>


class CameraComponent;

class FixedCamera : public Entity
{
public:
	FixedCamera();
	virtual ~FixedCamera();

protected:

	virtual void Init();

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

