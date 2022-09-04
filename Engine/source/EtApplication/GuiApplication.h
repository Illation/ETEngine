#pragma once
#include <RmlUi/Core/ElementInstancer.h>

#include <EtCore/UpdateCycle/Context.h>
#include <EtCore/Util/WindowSettings.h>

#include <EtApplication/GUI/ElementWindowHandle.h>


// fwd
namespace Rml {
	class Context;
}
namespace et { namespace app {
	class GuiWindow;
} }


namespace et {
namespace app {


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

	// accessors
	//-----------
	Ptr<GuiWindow> GetWindow(Rml::Context const* const context);

	// utility
	//---------
private:
	void CloseMarkedWindows();


	// Data
	///////

	std::vector<UniquePtr<GuiWindow>> m_Windows;
	std::vector<size_t> m_WindowsToDelete;

	core::BaseContext m_Context;

	// Gui types
	Rml::ElementInstancerGeneric<ElementWindowHandle> m_HandleInstancer;
};


} // namespace app
} // namespace et

