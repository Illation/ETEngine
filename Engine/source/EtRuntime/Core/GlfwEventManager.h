#pragma once
#include <map>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <EtCore/UpdateCycle/Tickable.h>
#include <EtCore/Util/Singleton.h>

#include <EtFramework/Config/TickOrder.h>

#include <EtRuntime/Rendering/GlfwRenderArea.h>


namespace et {
namespace rt {


//----------------------------
// GlfwEventManager
//
// Responsible for passing GLFW events to the input manager
//
class GlfwEventManager : public core::Singleton<GlfwEventManager>, public core::I_Tickable, public core::I_CursorShapeManager
{
private:
	// definitions
	//--------------
	friend class AbstractFramework;
public:
	// ctor dtor
	//---------------
	GlfwEventManager() : core::I_Tickable(static_cast<uint32>(fw::E_TickOrder::TICK_GlfwEventManager)) {}
	virtual ~GlfwEventManager();

	void Init(GlfwRenderArea* const renderArea);

	// modify state
	//--------------
protected:
	bool OnCursorResize(core::E_CursorShape const shape) override;
private:
	void OnTick() override; // call before all GUI ticks

	// utility
	//--------------
	static E_MouseButton GetButtonFromGlfw(int32 const bottonCode);

	// Data 
	/////////

	// Cursors
	std::map<core::E_CursorShape, GLFWcursor*> m_CursorMap;
	GlfwRenderArea* m_RenderArea = nullptr;
};


} // namespace rt
} // namespace et
