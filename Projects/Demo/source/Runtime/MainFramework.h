#pragma once
#include "FreeCamera.h"

#include <EtRuntime/AbstractFramework.h>


//--------------------------
// MainFramework
//
// User facing wrapper around the engine
//
class MainFramework final : public AbstractFramework
{
	// construct destruct
	//--------------------
public:
	MainFramework() : AbstractFramework() {} // initializes the engine
	~MainFramework() = default;

	// framework interface
	//---------------------
private:
	void AddScenes() override;
	void OnTick() override;

	// Data
	///////

	FreeCamera m_CameraController;
};

