#pragma once


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
	void WaitForEvents(uint64 const timeout);
	void Draw();

	void RegisterWindow(Ptr<GuiWindow> const window);
	void UnregisterWindow(GuiWindow const* const window);


	// Data
	///////

private:
	std::vector<Ptr<GuiWindow>> m_Windows;
};


} // namespace app
} // namespace et

