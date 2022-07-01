#include "stdafx.h"
#include "DemoUI.h"

#include <EtGUI/Context/RmlGlobal.h>


namespace et {
namespace demo {


//=========
// Demo UI
//=========


// static
core::HashString const DemoUI::s_HelloWorldGuiId("GUI/hello_world.rml");


//--------------------------
// DemoUI::SetupDataModels
//
// Ensure data models for GUI canvases we use are present when the components load
//
void DemoUI::SetupDataModels()
{
	gui::RmlGlobal::GetDataModelFactory().RegisterInstancer("animals", gui::DataModelFactory::T_InstanceFn(
		[](Rml::DataModelConstructor modelConstructor) -> RefPtr<gui::I_DataModel>
		{
			RefPtr<GuiData> ret = Create<GuiData>();
			modelConstructor.Bind("show_text", &ret->m_ShowText);
			modelConstructor.Bind("animal", &ret->m_Animal);
			return std::move(ret);
		}));
}


} // namespace demo
} // namespace et
