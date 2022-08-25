#pragma once
#include <EtCore/UpdateCycle/Tickable.h>
#include <EtCore/Util/Singleton.h>

#include <EtFramework/Config/TickOrder.h>

#include "GlfwEventBase.h"


namespace et {
namespace app {


//----------------------------
// GlfwEventManager
//
// Responsible for passing GLFW events to the input manager
//
class GlfwEventManager : public core::Singleton<GlfwEventManager>, 
	public core::I_Tickable, 
	public GlfwEventBase
{
private:
	// definitions
	//--------------
	friend class AbstractFramework;
public:
	// ctor dtor
	//---------------
	GlfwEventManager() : core::I_Tickable(static_cast<uint32>(fw::E_TickOrder::TICK_GlfwEventManager)), GlfwEventBase() {}
	virtual ~GlfwEventManager() = default;

	void Init(Ptr<GlfwRenderArea> const renderArea);

	// modify state
	//--------------
private:
	void OnTick() override; // call before all GUI ticks
};


} // namespace app
} // namespace et
