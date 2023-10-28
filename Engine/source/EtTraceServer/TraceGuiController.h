#pragma once

#include <EtGUI/Context/DataModel.h>
#include <EtApplication/GuiWindow.h>

#include <RmlUi/Core/DataModelHandle.h>
#include <RmlUi/Core/Event.h>
#include <RmlUi/Core/EventListener.h>
#include <RmlUi/Core/Types.h>


// fwd
namespace Rml {
	class ElementTabSet;
}

namespace et { namespace app {
	class GuiApplication;
} namespace trace {
	class TraceGuiController;
} }


namespace et {
namespace trace {


//----------
// GuiData
//
// Data model for the trace server UI
//
struct GuiData : public gui::I_DataModel
{
	//definitions
	//------------
	struct Panel
	{
		std::string m_Name;
		std::string m_IdRef;

		std::vector<std::string> m_Lines;
	};

	// static functionality
	//---------------------
	static void RegisterInstancer();

	// construct destruct
	//-------------------
	void Init(Ptr<TraceGuiController> controller) { m_Controller = controller; }

	// Data
	///////

	// reflected
	std::string m_WindowTitle;
	bool m_ShowOptions = false;
	std::vector<Panel> m_Panels;

	// utility
private:
	Ptr<TraceGuiController> m_Controller;
};

//--------------------
// TraceGuiController
//
// Data model for the trace server UI
//
class TraceGuiController : public Rml::EventListener
{
public:
	// construct destruct
	//--------------------
	void Initialize(app::GuiApplication* const guiApp);

	// functionality
	//---------------
	void CreatePanel();

	// utility
	//---------
protected:
	void ProcessEvent(Rml::Event& evnt) override;


	// Data
	///////
private:
	Ptr<app::GuiWindow> m_MainWindow;
	WeakPtr<GuiData> m_DataModel;

	Ptr<Rml::ElementTabSet> m_TabSet;
	Ptr<Rml::Element> m_TabsContainer;
};


} // namespace trace
} // namespace et
