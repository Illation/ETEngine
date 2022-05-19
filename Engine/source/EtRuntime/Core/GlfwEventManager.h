#pragma once
#include <map>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <EtCore/UpdateCycle/Tickable.h>
#include <EtCore/Util/Singleton.h>
#include <EtCore/Input/RawInputProvider.h>

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

	void Init(Ptr<GlfwRenderArea> const renderArea);

	// accessors
	//-----------
	core::RawInputProvider& GetInputProvider() { return m_InputProvider; }

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

	std::map<core::E_CursorShape, GLFWcursor*> m_CursorMap;
	Ptr<GlfwRenderArea> m_RenderArea;
	core::RawInputProvider m_InputProvider;
};


} // namespace rt
} // namespace et
