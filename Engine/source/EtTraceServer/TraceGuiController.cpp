#include "stdafx.h"
#include "TraceGuiController.h"

#include <RmlUi/Core/Factory.h>
#include <RmlUi/Core/Elements/ElementTabSet.h>
#include <RmlUi/Core/ElementDocument.h>
#include <RmlUi/Core/ElementUtilities.h>

#include <EtGUI/Context/RmlGlobal.h>
#include <EtGUI/Context/Context.h>
#include <EtGUI/Context/RmlUtil.h>

#include <EtApplication/GuiApplication.h>


namespace et {
namespace trace {


//==========
// GUI Data
//==========


static char const* const s_WindowTitleId = "td_window_title";
static char const* const s_PanelsDataId = "td_trace_panels";
static char const* const s_ActivePanelId = "td_active_panel";
static core::HashString const s_MainDocumentId("trace.rml");
static float const s_TabsScrollSpeed = 30.f;
	

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

			if (Rml::StructHandle<Panel::ContextFilter> contextFilterHandle = modelConstructor.RegisterStruct<Panel::ContextFilter>())
			{
				ET_CHECK_W(contextFilterHandle.RegisterMember("name", &Panel::ContextFilter::m_ContextName));
				ET_CHECK_W(contextFilterHandle.RegisterMember("show", &Panel::ContextFilter::m_Show));
			}
			else
			{
				ET_WARNING("Failed to register Context Filter Struct");
			}

			modelConstructor.RegisterArray<std::vector<Panel::ContextFilter>>();

			if (Rml::StructHandle<Panel> panelHandle = modelConstructor.RegisterStruct<Panel>())
			{
				ET_CHECK_W(panelHandle.RegisterMember("id", &Panel::m_Id));
				ET_CHECK_W(panelHandle.RegisterMember("name", &Panel::m_Name));

				ET_CHECK_W(panelHandle.RegisterMember("lines", &Panel::m_Lines));

				ET_CHECK_W(panelHandle.RegisterMember("filter_text", &Panel::m_SearchText));

				ET_CHECK_W(panelHandle.RegisterMember("show_info", &Panel::m_ShowInfo));
				ET_CHECK_W(panelHandle.RegisterMember("show_verbose", &Panel::m_ShowVerbose));
				ET_CHECK_W(panelHandle.RegisterMember("show_warning", &Panel::m_ShowWarning));
				ET_CHECK_W(panelHandle.RegisterMember("show_error", &Panel::m_ShowError));
				ET_CHECK_W(panelHandle.RegisterMember("show_fatal", &Panel::m_ShowFatal));

				ET_CHECK_W(panelHandle.RegisterMember("contexts", &Panel::m_Contexts));
			}
			else
			{
				ET_WARNING("Failed to register Panel Struct");
			}

			gui::RmlUtil::RegisterSlotMapToDataModelConstructor<T_PanelMap>(modelConstructor);

			// bind functions
			modelConstructor.BindEventCallback("CreatePanel", [data = ret.Get()](Rml::DataModelHandle, Rml::Event&, Rml::VariantList const&)
				{
					data->m_Controller->CreatePanel(); 
				});
			modelConstructor.BindEventCallback("ClosePanel", [data = ret.Get()](Rml::DataModelHandle, Rml::Event& evnt, Rml::VariantList const& params)
				{
					ET_ASSERT(params.size() == 1u, "Expect ClosePanel to be called with one parameter");

					core::T_SlotId id;
					ET_CHECK_W(params[0].GetInto(id), "expected an index type in ClosePanel param 0");

					data->m_Controller->ClosePanel(id);

					evnt.StopPropagation();
				});

			// bind data
			modelConstructor.Bind(s_WindowTitleId, &ret->m_WindowTitle);
			modelConstructor.Bind("td_show_options", &ret->m_ShowOptions);
			modelConstructor.Bind(s_PanelsDataId, &ret->m_Panels);
			modelConstructor.Bind(s_ActivePanelId, &ret->m_ActivePanel);

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
	// Gui Setup
	GuiData::RegisterInstancer();

	// Main Window
	core::WindowSettings settings;
	settings.m_Title = "E.T. Trace Server";
	settings.m_Decorated = false; // we use a custom window handle
	settings.m_Resolutions.emplace_back(1280, 720);
	m_MainWindow = guiApp->MakeWindow(settings);
	m_MainWindow->SetGuiDocument(s_MainDocumentId);
	m_MainWindow->SetIcon(core::HashString("trace_logo_colour_thick.svg"));
	
	// Data model
	m_DataModel = WeakPtr<GuiData>::StaticCast(m_MainWindow->GetContext().GetDataModel(s_MainDocumentId));
	m_DataModel->Init(ToPtr(this));

	m_DataModel->m_WindowTitle = settings.m_Title;
	m_DataModel->m_ModelHandle.DirtyVariable(s_WindowTitleId);

	// Elements
	m_TabSet = ToPtr(static_cast<Rml::ElementTabSet*>(m_MainWindow->GetContext().GetDocument(s_MainDocumentId)->GetElementById("trace_panels")));
	ET_ASSERT(m_TabSet != nullptr);

	m_TabSet->SetActiveTab(-1);

	Rml::ElementList panelsEls;
	m_TabSet->GetElementsByTagName(panelsEls, "panels");
	ET_ASSERT(panelsEls.size() == 1u);
	m_TabSetPanels = ToPtr(panelsEls[0]);

	UpdateActivePanelId();

	m_TabsContainer = ToPtr(m_MainWindow->GetContext().GetDocument(s_MainDocumentId)->GetElementById("trace_panel_tabs"));
	ET_ASSERT(m_TabsContainer != nullptr);

	m_TabsContainer->AddEventListener(Rml::EventId::Mousescroll, this, true);
	m_TabSet->AddEventListener(Rml::EventId::Tabchange, this, true);
}

//----------------------------
// TraceGuiController::Update
//
void TraceGuiController::Update()
{
	for (core::T_SlotId const panelId : m_PanelsToDelete)
	{
		GuiData::T_PanelMap& panels = m_DataModel->m_Panels;
		ET_ASSERT(panels.is_valid(panelId));

		int panelIdx = -1;
		{
			Rml::Element const* panelEl = panels[panelId].m_PanelEl.Get();
			ET_ASSERT(panelEl->GetParentNode()->GetTagName() == "panels");
			while (panelEl != nullptr)
			{
				panelEl = panelEl->GetPreviousSibling();
				panelIdx++;
			}
		}

		bool const isActiveTab = (panelIdx == m_TabSet->GetActiveTab());
		m_TabSet->RemoveTab(panelIdx);
		m_DataModel->m_Panels.erase(panelId);

		m_DataModel->m_ModelHandle.DirtyVariable(s_PanelsDataId);

		if (isActiveTab)
		{
			if (panelIdx > 0)
			{
				m_TabSet->SetActiveTab(panelIdx - 1);
			}
			else if (m_TabSet->GetNumTabs() > 0)
			{
				m_TabSet->SetActiveTab(0);
			}
		}
	}

	m_PanelsToDelete.clear();
}

//---------------------------------
// TraceGuiController::CreatePanel
//
void TraceGuiController::CreatePanel()
{
	int const panelIdx = static_cast<int>(m_DataModel->m_Panels.size()); // only for addressing the tabset - internally we use panel IDs from the slot map

	// update date model
	ET_ASSERT(m_DataModel != nullptr);

	std::pair<GuiData::T_PanelMap::iterator, core::T_SlotId> inserted = m_DataModel->m_Panels.insert(GuiData::Panel());
	GuiData::Panel& panel = *inserted.first;

	panel.m_Id = inserted.second;

	uint32 const panelNumber = panel.m_Id + 1;
	panel.m_Name = FS("Client %u", panelNumber);
	for (uint32 lineIdx = 0u; lineIdx < panelNumber; ++lineIdx)
	{
		panel.m_Lines.emplace_back(FS("Panel %u - Line %u", panelNumber, lineIdx));
	}

	// ensure update
	m_DataModel->m_ModelHandle.DirtyVariable(s_PanelsDataId);

	// instantiating panels
	Rml::XMLAttributes attributes;
	attributes.emplace("data-alias-panel_ref", FS("%s[%u]", s_PanelsDataId, panel.m_Id));

	attributes["class"] = "close_tab";
	Rml::Element* const insertedTab = m_TabSet->SetTab(panelIdx, Rml::Factory::InstanceElement(m_TabSet.Get(), "*", "tab", attributes));

	attributes["class"] = "trace_panel";
	panel.m_PanelEl = ToPtr(m_TabSet->SetPanel(panelIdx, Rml::Factory::InstanceElement(m_TabSet.Get(), "*", "panel", attributes)));

	m_TabSet->SetActiveTab(panelIdx);

	// we ensure that the element is already linked to the document structure so that the data bindings work correctly
	Rml::Element* templateContent;
	Rml::ElementUtilities::ParseTemplateIntoElement(panel.m_PanelEl.Get(), "template_trace_panel.rml", templateContent);

	Rml::ElementUtilities::ParseTemplateIntoElement(insertedTab, "template_close_tab.rml", templateContent);
	ET_ASSERT(templateContent != nullptr);
	Rml::Factory::InstanceElementText(templateContent, panel.m_Name);
}

//--------------------------------
// TraceGuiController::ClosePanel
//
void TraceGuiController::ClosePanel(core::T_SlotId const panelId)
{
	m_PanelsToDelete.push_back(panelId);
}

//----------------------------------
// TraceGuiController::ProcessEvent
//
void TraceGuiController::ProcessEvent(Rml::Event& evnt)
{
	if (evnt.GetCurrentElement() == m_TabsContainer.Get())
	{
		if ((evnt.GetId() == Rml::EventId::Mousescroll) && !evnt.GetParameter("autoscroll", false))
		{
			float const delta = evnt.GetParameter("wheel_delta_y", 0.f) * s_TabsScrollSpeed;
			float const newLeft = math::Clamp(m_TabsContainer->GetScrollLeft() + delta, 
				std::max(m_TabsContainer->GetScrollWidth() - m_TabsContainer->GetClientWidth(), 0.f),
				0.f);

			m_TabsContainer->SetScrollLeft(newLeft);
		}
	}
	else if (evnt.GetCurrentElement() == m_TabSet.Get())
	{
		if (evnt.GetId() == Rml::EventId::Tabchange)
		{
			UpdateActivePanelId();
		}
	}
}

//-----------------------------------------
// TraceGuiController::UpdateActivePanelId
//
void TraceGuiController::UpdateActivePanelId()
{
	int const activeTabIdx = m_TabSet->GetActiveTab();
	if ((activeTabIdx >= m_TabSetPanels->GetNumChildren()) || (activeTabIdx < 0))
	{
		m_DataModel->m_ActivePanelId = core::INVALID_SLOT_ID;
		m_DataModel->m_ActivePanel = &m_DataModel->m_FallbackPanel;
	}
	else
	{
		Rml::Element const* const panelEl = m_TabSetPanels->GetChild(activeTabIdx);
		ET_ASSERT(panelEl != nullptr);

		auto const foundPanelIt = std::find_if(m_DataModel->m_Panels.cbegin(), m_DataModel->m_Panels.cend(), [panelEl](GuiData::Panel const& panel)
			{
				return panel.m_PanelEl.Get() == panelEl;
			});
		ET_ASSERT(foundPanelIt != m_DataModel->m_Panels.cend());

		m_DataModel->m_ActivePanelId = m_DataModel->m_Panels.iterator_id(foundPanelIt);
		m_DataModel->m_ActivePanel = &m_DataModel->m_Panels[m_DataModel->m_ActivePanelId];
	}

	m_DataModel->m_ModelHandle.DirtyVariable(s_ActivePanelId);
}


} // namespace trace
} // namespace et
