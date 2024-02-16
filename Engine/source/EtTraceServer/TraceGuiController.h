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
		// reflected
		core::T_SlotId m_Id; // there seems to be no other way to reverse identify panels from data expressions

		std::string m_Name;
		std::vector<std::string> m_Lines;

		// utility
		Ptr<Rml::Element> m_PanelEl;
	};

	typedef core::slot_map<Panel> T_PanelMap;

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
	T_PanelMap m_Panels;

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
	void Update();

	// functionality
	//---------------
	void CreatePanel();
	void ClosePanel(core::T_SlotId const panelId);

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

	std::vector<core::T_SlotId> m_PanelsToDelete;
};


} // namespace trace
} // namespace et
