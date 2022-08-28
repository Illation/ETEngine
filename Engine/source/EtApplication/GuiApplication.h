#pragma once
#include <EtCore/UpdateCycle/Context.h>


namespace et {
namespace app {


// fwd
class GuiWindow;


//----------------
// GuiApplication
//
class GuiApplication 
{
	// construct destruct
	//--------------------
public:
	GuiApplication() = default;
	virtual ~GuiApplication() = default;

	// functionality
	//---------------
	void ReceiveEvents(int64 const timeout);
	void Draw();

	void RegisterWindow(Ptr<GuiWindow> const window);
	void UnregisterWindow(GuiWindow const* const window);


	// Data
	///////

private:
	std::vector<Ptr<GuiWindow>> m_Windows;

	core::BaseContext m_Context;
};


} // namespace app
} // namespace et

