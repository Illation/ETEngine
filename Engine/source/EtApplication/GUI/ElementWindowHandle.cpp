#include "stdafx.h"
#include "ElementWindowHandle.h"

#include <RmlUi/Core/Factory.h>
#include <RmlUi/Core/ElementUtilities.h>
#include <RmlUi/SVG/ElementSVG.h>

#include <EtApplication/GuiApplication.h>
#include <EtApplication/GuiWindow.h>


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


//----------------------------
// ElementWindowHandle::c-tor
//
ElementWindowHandle::ElementWindowHandle(Rml::String const& tag)
	: Rml::Element(tag)
{
	// Make sure we can be dragged!
	SetProperty(Rml::PropertyId::Drag, Rml::Property(Rml::Style::Drag::Drag));
	SetProperty(Rml::PropertyId::ZIndex, Rml::Property(1, Rml::Property::Unit::NUMBER));

	// Create buttons
	m_ControlsArea = ToPtr(AppendChild(Rml::Factory::InstanceElement(this, "*", "controls", Rml::XMLAttributes()), false));
	m_ControlsArea->SetProperty(Rml::PropertyId::ZIndex, Rml::Property(2, Rml::Property::Unit::NUMBER));

	static float const s_IconSize = 26.f;
	Rml::XMLAttributes attributes;
	attributes.emplace("width", s_IconSize);
	attributes.emplace("height", s_IconSize);

	attributes["src"] = "icons/minimize.svg";
	m_MinButton = ToPtr(m_ControlsArea->AppendChild(Rml::Factory::InstanceElement(m_ControlsArea.Get(), "svg", "min-button", attributes), false));
	attributes["src"] = "icons/restore.svg";
	m_MaxButton = ToPtr(m_ControlsArea->AppendChild(Rml::Factory::InstanceElement(m_ControlsArea.Get(), "svg", "max-button", attributes), false));
	attributes["src"] = "icons/close.svg";
	m_CloseButton = ToPtr(m_ControlsArea->AppendChild(Rml::Factory::InstanceElement(m_ControlsArea.Get(), "svg", "close-button", attributes), false));

	// Setup events
	m_Listener.SetHandle(ToPtr(this));

	m_MinButton->AddEventListener(Rml::EventId::Click, &m_Listener);
	m_MaxButton->AddEventListener(Rml::EventId::Click, &m_Listener);
	m_CloseButton->AddEventListener(Rml::EventId::Click, &m_Listener);

	FormatChildren();
}

//-------------------------------
// ElementWindowHandle::LazyInit
//
void ElementWindowHandle::LazyInit()
{
	if (m_Window == nullptr)
	{
		m_Window = GuiApplication::Instance()->GetWindow(GetContext());
		ET_ASSERT(m_Window != nullptr);
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

//------------------------------------
// ElementWindowHandle::OnButtonEvent
//
void ElementWindowHandle::FormatChildren()
{
	Rml::Vector2f const dim = GetBox().GetSize(Rml::Box::CONTENT);

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


} // namespace app
} // namespace et

