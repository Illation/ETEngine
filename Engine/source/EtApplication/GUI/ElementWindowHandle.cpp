#include "stdafx.h"
#include "ElementWindowHandle.h"

#include <RmlUi/Core/Factory.h>
#include <RmlUi/Core/ElementUtilities.h>
#include <RmlUi/SVG/ElementSVG.h>

#include <EtApplication/GuiApplication.h>


namespace et {
namespace app {


//===================================
// Window Handle Element :: Listener
//===================================


//------------------------------
// WindowListener::ProcessEvent
//
void ElementWindowHandle::WindowListener::ProcessEvent(Rml::Event& evnt)
{
	m_WindowHandle->OnButtonEvent(evnt);
}


//=======================
// Window Handle Element
//=======================


// static
float const ElementWindowHandle::s_Height = 32.f;
std::string const ElementWindowHandle::s_IconAttribId("icon");


//----------------------------
// ElementWindowHandle::c-tor
//
ElementWindowHandle::ElementWindowHandle(Rml::String const& tag)
	: Rml::Element(tag)
{
	// Make sure we can be dragged!
	SetProperty(Rml::PropertyId::Drag, Rml::Property(Rml::Style::Drag::Drag));
	SetProperty(Rml::PropertyId::ZIndex, Rml::Property(1, Rml::Property::Unit::NUMBER));
	SetProperty(Rml::PropertyId::Height, Rml::Property(s_Height, Rml::Property::Unit::NUMBER));

	// Create buttons
	m_ControlsArea = ToPtr(AppendChild(Rml::Factory::InstanceElement(this, "*", "controls", Rml::XMLAttributes()), false));
	m_ControlsArea->SetProperty(Rml::PropertyId::ZIndex, Rml::Property(2, Rml::Property::Unit::NUMBER));

	Rml::XMLAttributes attributes;
	attributes.emplace("width", s_Height);
	attributes.emplace("height", s_Height);

	attributes["src"] = "icons/minimize.svg";
	m_MinButton = ToPtr(m_ControlsArea->AppendChild(Rml::Factory::InstanceElement(m_ControlsArea.Get(), "svg", "min-button", attributes), false));
	attributes["src"] = "icons/restore.svg";
	m_MaxButton = ToPtr(m_ControlsArea->AppendChild(Rml::Factory::InstanceElement(m_ControlsArea.Get(), "svg", "max-button", attributes), false));
	attributes["src"] = "icons/close.svg";
	m_CloseButton = ToPtr(m_ControlsArea->AppendChild(Rml::Factory::InstanceElement(m_ControlsArea.Get(), "svg", "close-button", attributes), false));

	InitIcon();

	// Setup events
	m_Listener.SetHandle(ToPtr(this));

	m_MinButton->AddEventListener(Rml::EventId::Click, &m_Listener);
	m_MaxButton->AddEventListener(Rml::EventId::Click, &m_Listener);
	m_CloseButton->AddEventListener(Rml::EventId::Click, &m_Listener);

	FormatChildren();
}

//----------------------------
// ElementWindowHandle::d-tor
//
ElementWindowHandle::~ElementWindowHandle()
{
	if (m_WindowCallbackId != GuiWindow::INVALID_CALLBACK)
	{
		ET_ASSERT(m_Window != nullptr);
		m_Window->UnregisterCallback(m_WindowCallbackId);
	}
}

//-------------------------------
// ElementWindowHandle::LazyInit
//
void ElementWindowHandle::LazyInit()
{
	if (!m_IsInitialized)
	{
		m_Window = GuiApplication::Instance()->GetWindow(GetContext());
		ET_ASSERT(m_Window != nullptr);

		m_WindowCallbackId = m_Window->RegisterCallback(GuiWindow::GW_All, 
			GuiWindow::T_EventCallback([this](GuiWindow::T_EventFlags const evnt, GuiWindow::EventData const* const)
			{
				switch (static_cast<GuiWindow::E_Event>(evnt))
				{
				case GuiWindow::GW_GainFocus:
					OnFocusChange(true);
					break;
				case GuiWindow::GW_LooseFocus:
					OnFocusChange(false);
					break;
				case GuiWindow::GW_Maximize:
					OnMaximizedChanged(true);
					break;
				case GuiWindow::GW_Restore:
					OnMaximizedChanged(false);
					break;
				}
			}));

		InitIcon();

		OnFocusChange(m_Window->Focused());
		OnMaximizedChanged(m_Window->Maximized());

		m_IsInitialized = true;
	}
}


//-------------------------------------------
// ElementWindowHandle::OnAttributeChange
//
void ElementWindowHandle::OnAttributeChange(Rml::ElementAttributes const& changedAttributes)
{
	Rml::Element::OnAttributeChange(changedAttributes);

	if (changedAttributes.find(s_IconAttribId) != changedAttributes.cend())
	{
		m_IsInitialized = false;
	}
}

//-------------------------------------------
// ElementWindowHandle::ProcessDefaultAction
//
void ElementWindowHandle::ProcessDefaultAction(Rml::Event& evnt)
{
	Rml::Element::ProcessDefaultAction(evnt);

	if (evnt.GetTargetElement() == this)
	{
		LazyInit();

		if (evnt == Rml::EventId::Dragstart)
		{
			m_Window->StartDrag();
		}
	}
}

//-------------------------------
// ElementWindowHandle::OnLayout
//
void ElementWindowHandle::OnLayout()
{
	Rml::Element::OnLayout();
	LazyInit();
	FormatChildren();
}

//-------------------------------
// ElementWindowHandle::OnResize
//
void ElementWindowHandle::OnResize()
{
	Rml::Element::OnResize();
	FormatChildren();
}

//------------------------------------
// ElementWindowHandle::OnButtonEvent
//
void ElementWindowHandle::OnButtonEvent(Rml::Event& evnt)
{
	if (!(evnt == Rml::EventId::Click))
	{
		return;
	}

	LazyInit();

	if (evnt.GetCurrentElement() == m_MinButton)
	{
		m_Window->ToggleMinimized();
	}
	else if (evnt.GetCurrentElement() == m_MaxButton)
	{
		m_Window->ToggleMaximized();
	}
	else if (evnt.GetCurrentElement() == m_CloseButton)
	{
		m_Window->Close();
	}
}

//-------------------------------------
// ElementWindowHandle::FormatChildren
//
void ElementWindowHandle::FormatChildren()
{
	Rml::Vector2f const dim = GetBox().GetSize(Rml::Box::CONTENT);

	if (m_Icon != nullptr)
	{
		Rml::ElementUtilities::PositionElement(m_Icon.Get(), Rml::Vector2f(0, 0), Rml::ElementUtilities::TOP_LEFT);
		Rml::ElementUtilities::FormatElement(m_Icon.Get(), Rml::Vector2f(dim.y, dim.y));
	}

	float const controlW = dim.y * 3.f;
	Rml::ElementUtilities::PositionElement(m_ControlsArea.Get(), Rml::Vector2f(controlW -dim.x, 0), Rml::ElementUtilities::TOP_RIGHT);
	Rml::ElementUtilities::FormatElement(m_ControlsArea.Get(), Rml::Vector2f(controlW, dim.y));


	Rml::ElementUtilities::PositionElement(m_MinButton.Get(), Rml::Vector2f(0, 0), Rml::ElementUtilities::TOP_LEFT);
	Rml::ElementUtilities::FormatElement(m_MinButton.Get(), Rml::Vector2f(dim.y, dim.y));

	Rml::ElementUtilities::PositionElement(m_MaxButton.Get(), Rml::Vector2f(dim.y, 0), Rml::ElementUtilities::TOP_LEFT);
	Rml::ElementUtilities::FormatElement(m_MaxButton.Get(), Rml::Vector2f(dim.y, dim.y));

	Rml::ElementUtilities::PositionElement(m_CloseButton.Get(), Rml::Vector2f(dim.y * 2, 0), Rml::ElementUtilities::TOP_LEFT);
	Rml::ElementUtilities::FormatElement(m_CloseButton.Get(), Rml::Vector2f(dim.y, dim.y));
}

//-------------------------------------
// ElementWindowHandle::InitIcon
//
void ElementWindowHandle::InitIcon()
{
	if (m_Icon != nullptr)
	{
		RemoveChild(m_Icon.Get());
		m_Icon = nullptr;
		DirtyLayout();
	}

	std::string const iconId = GetAttribute<std::string>(s_IconAttribId, "");
	if (!iconId.empty())
	{
		Rml::XMLAttributes attributes;
		attributes.emplace("width", s_Height);
		attributes.emplace("height", s_Height);
		attributes["src"] = iconId;
		m_Icon = ToPtr(AppendChild(Rml::Factory::InstanceElement(this, "svg", "icon", attributes), false));
		DirtyLayout();
	}
}

//-------------------------------------
// ElementWindowHandle::OnFocusChange
//
void ElementWindowHandle::OnFocusChange(bool const focused)
{
	SetPseudoClass("window_focus", focused);
}

//-----------------------------------------
// ElementWindowHandle::OnMaximizedChanged
//
void ElementWindowHandle::OnMaximizedChanged(bool const maximized)
{
	m_MaxButton->SetAttribute("src", maximized ? "icons/restore.svg" : "icons/maximize.svg");
	// #todo: there is a bug in the RML ui implementation that means that the texture only updates if the image resizes
	m_MaxButton->SetAttribute("height", s_Height + (maximized ? 1 : -1)); 
}


} // namespace app
} // namespace et

