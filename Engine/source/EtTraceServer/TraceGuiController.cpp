#include "stdafx.h"
#include "TraceGuiController.h"

#include <RmlUi/Core/Factory.h>
#include <RmlUi/Core/Elements/ElementTabSet.h>
#include <RmlUi/Core/ElementDocument.h>
#include <RmlUi/Core/ElementUtilities.h>

#include <EtGUI/Context/RmlGlobal.h>
#include <EtGUI/Context/Context.h>

#include <EtApplication/GuiApplication.h>


namespace et {
namespace trace {


//==========
// GUI Data
//==========


static char const* const s_PanelsDataId = "td_trace_panels";
static core::HashString const s_MainDocumentId("trace.rml");
	

//----------------------------
// GuiData::RegisterInstancer
//
// Ensure data model for GUI is present on document load
//
void GuiData::RegisterInstancer()
{
	gui::RmlGlobal::GetDataModelFactory().RegisterInstancer("trace_data", gui::DataModelFactory::T_InstanceFn(
		[](Rml::DataModelConstructor modelConstructor) -> RefPtr<gui::I_DataModel>
		{
			RefPtr<GuiData> ret = Create<GuiData>();

			// register types
			modelConstructor.RegisterArray<std::vector<std::string>>();

			if (Rml::StructHandle<Panel> panelHandle = modelConstructor.RegisterStruct<Panel>())
			{
				ET_CHECK_W(panelHandle.RegisterMember("name", &Panel::m_Name));
				ET_CHECK_W(panelHandle.RegisterMember("id", &Panel::m_IdRef));
				ET_CHECK_W(panelHandle.RegisterMember("lines", &Panel::m_Lines));
			}
			else
			{
				ET_WARNING("Failed to register Panel Struct");
			}

			modelConstructor.RegisterArray<std::vector<Panel>>();

			// bind functions
			modelConstructor.BindEventCallback("CreatePanel", [data = ret.Get()](Rml::DataModelHandle, Rml::Event&, Rml::VariantList const&)
				{
					data->m_Controller->CreatePanel(); 
				});

			// bind data
			modelConstructor.Bind("show_options", &ret->m_ShowOptions);
			modelConstructor.Bind(s_PanelsDataId, &ret->m_Panels);

			return std::move(ret);
		}));
}


//======================
// Trace GUI Controller
//======================


//---------------------------------
// TraceGuiController::CreatePanel
//
void TraceGuiController::Initialize(app::GuiApplication* const guiApp)
{
	GuiData::RegisterInstancer();

	core::WindowSettings settings;
	settings.m_Title = "E.T. Trace Server";
	settings.m_Decorated = false; // we use a custom window handle
	settings.m_Resolutions.emplace_back(1280, 720);
	m_MainWindow = guiApp->MakeWindow(settings);
	m_MainWindow->SetGuiDocument(s_MainDocumentId);
	m_MainWindow->SetIcon(core::HashString("trace_logo_colour_thick.svg"));
	
	m_DataModel = WeakPtr<GuiData>::StaticCast(m_MainWindow->GetContext().GetDataModel(s_MainDocumentId));
	m_DataModel->Init(ToPtr(this));

	m_TabSet = ToPtr(static_cast<Rml::ElementTabSet*>(m_MainWindow->GetContext().GetDocument(s_MainDocumentId)->GetElementById("trace_panels")));
}

//---------------------------------
// TraceGuiController::CreatePanel
//
void TraceGuiController::CreatePanel()
{
	size_t const panelIdx = m_DataModel->m_Panels.size();

	// update date model
	ET_ASSERT(m_DataModel != nullptr);

	m_DataModel->m_Panels.emplace_back();
	GuiData::Panel& panel = m_DataModel->m_Panels.back();
	panel.m_Name = FS("Client "ET_FMT_SIZET, panelIdx + 1u);
	panel.m_IdRef = FS("client"ET_FMT_SIZET, panelIdx + 1u);
	for (size_t lineIdx = 0u; lineIdx < panelIdx + 1u; ++lineIdx)
	{
		panel.m_Lines.emplace_back(FS("Line " ET_FMT_SIZET, lineIdx));
	}

	// ensure update
	m_DataModel->m_ModelHandle.DirtyVariable(s_PanelsDataId);

	// instantiating panels
	Rml::XMLAttributes attributes;
	attributes.emplace("class", "trace_panel");
	attributes.emplace("data-alias-panel_ref", FS("%s[" ET_FMT_SIZET "]", s_PanelsDataId, panelIdx));

	m_TabSet->SetTab(static_cast<int>(panelIdx), FS("Client %u", panelIdx + 1u));
	Rml::Element* const insertedPanel = m_TabSet->SetPanel(static_cast<int>(panelIdx), 
		Rml::Factory::InstanceElement(m_TabSet.Get(), "*", "panel", attributes));

	// we ensure that the element is already linked to the document structure so that the data bindings work correctly
	Rml::ElementUtilities::ParseTemplateIntoElement(insertedPanel, "template_trace_panel.rml");
}


} // namespace trace
} // namespace et
