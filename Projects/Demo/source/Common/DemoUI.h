#pragma once
#include <EtGUI/Context/DataModel.h>


namespace et {
namespace demo {


//--------------------------
// DemoUI
//
// Common class data for UI in the demo
//
class DemoUI final
{
public:
	static core::HashString const s_HelloWorldGuiId;

	//----------
	// GuiData
	//
	// Data model for the hello world screen
	//
	struct GuiData : public gui::I_DataModel
	{
		bool m_ShowText = true;
		std::string m_Animal = "dog";
	};

	static void SetupDataModels();
};


} // namespace demo
} // namespace et
