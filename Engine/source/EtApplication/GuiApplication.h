#pragma once
#include <EtCore/UpdateCycle/Context.h>
#include <EtCore/Util/WindowSettings.h>


namespace et {
namespace app {


// fwd
class GuiWindow;


//----------------
// GuiApplication
//
class GuiApplication 
{
	static Ptr<GuiApplication> s_GlobalInstance;

	// static
	//--------
public:
	static bool HasRunningInstance();
	static GuiApplication* Instance();

	// construct destruct
	//--------------------
	GuiApplication() = default;
	virtual ~GuiApplication() = default;

	// functionality
	//---------------
	void ReceiveEvents(int64 const timeout);
	void Draw();

	Ptr<GuiWindow> MakeWindow(core::WindowSettings const& settings);
	void MarkWindowForClose(GuiWindow const* const window);

	// utility
	//---------
private:
	void CloseMarkedWindows();


	// Data
	///////

	std::vector<UniquePtr<GuiWindow>> m_Windows;
	std::vector<size_t> m_WindowsToDelete;

	core::BaseContext m_Context;
};


} // namespace app
} // namespace et

