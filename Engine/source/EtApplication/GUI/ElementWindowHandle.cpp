#include "stdafx.h"
#include "ElementWindowHandle.h"

#include <RmlUi/Core/Factory.h>
#include <RmlUi/Core/ElementUtilities.h>
#include <RmlUi/SVG/ElementSVG.h>

#include <EtApplication/GuiApplication.h>
#include <EtApplication/GuiWindow.h>

#include <EtGUI/Context/RmlUtil.h>

#include "ElementWindow.h"


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
std::string const ElementWindowHandle::s_HitIgnoreClassName("handle_hit_ignore");
std::string const ElementWindowHandle::s_HitIgnoreChildrenClassName("handle_hit_ignore_children");

static std::string const s_IconClassAttribId("icon-class");
static std::string const s_IconLvalAttribId("icon-lval");
static std::string const s_IconRvalAttribId("icon-rval");


//----------------------------
// ElementWindowHandle::c-tor
//
ElementWindowHandle::ElementWindowHandle(Rml::String const& tag)
	: Rml::Element(tag)
{
	// Make sure we can be dragged!
	SetProperty(Rml::PropertyId::Drag, Rml::Property(Rml::Style::Drag::Drag));
	SetProperty(Rml::PropertyId::ZIndex, Rml::Property(1, Rml::Property::Unit::NUMBER));
	SetProperty(Rml::PropertyId::Height, Rml::Property(s_Height, Rml::Property::Unit::PX));

	// Create buttons
	m_ControlsArea = ToPtr(AppendChild(Rml::Factory::InstanceElement(this, "*", "controls", Rml::XMLAttributes()), false));
	m_ControlsArea->SetProperty(Rml::PropertyId::ZIndex, Rml::Property(2, Rml::Property::Unit::NUMBER));

	m_HitTestIgnoreElements.push_back(m_ControlsArea);

	Rml::XMLAttributes attributes;
	attributes.emplace("width", s_Height);
	attributes.emplace("height", s_Height - 1);

	attributes["src"] = "icons/minimize.svg";
	m_MinButton = ToPtr(m_ControlsArea->AppendChild(Rml::Factory::InstanceElement(m_ControlsArea.Get(), "svg", "min-button", attributes), false));
	attributes["src"] = "icons/restore.svg";
	m_MaxButton = ToPtr(m_ControlsArea->AppendChild(Rml::Factory::InstanceElement(m_ControlsArea.Get(), "svg", "max-button", attributes), false));
	attributes["src"] = "icons/close.svg";
	m_CloseButton = ToPtr(m_ControlsArea->AppendChild(Rml::Factory::InstanceElement(m_ControlsArea.Get(), "svg", "close-button", attributes), false));

	InitIcon();

	// Setup events
	m_Listener.SetHandle(ToPtr(this));

	AddEventListener(Rml::EventId::Dblclick, &m_Listener);
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
		m_Window->SetHandleHitTestFn(GuiWindow::T_WindowHandleHitTestFn(nullptr));
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

		m_WindowCallbackId = m_Window->RegisterCallback(GuiWindow::GW_Maximize | GuiWindow::GW_Restore,
			GuiWindow::T_EventCallback([this](GuiWindow::T_EventFlags const evnt, GuiWindow::EventData const* const)
			{
				switch (static_cast<GuiWindow::E_Event>(evnt))
				{
				case GuiWindow::GW_Maximize:
					OnMaximizedChanged(true);
					break;
				case GuiWindow::GW_Restore:
					OnMaximizedChanged(false);
					break;
				}
			}));

		m_Window->SetHandleHitTestFn(GuiWindow::T_WindowHandleHitTestFn([this](ivec2 const point)
			{
				Rml::Vector2f const p = gui::RmlUtil::ToRml(math::vecCast<float>(point));
				if (IsPointWithinElement(p))
				{
					for (Ptr<Rml::Element> const el : m_HitTestIgnoreElements)
					{
						if (el->IsPointWithinElement(p))
						{
							return false;
						}
					}

					for (Ptr<Rml::Element> const el : m_HitTestIgnoreChildren)
					{
						if (el->IsPointWithinElement(p))
						{
							for (int childIdx = 0; childIdx < el->GetNumChildren(); ++childIdx)
							{
								if (el->GetChild(childIdx)->IsPointWithinElement(p))
								{
									return false;
								}
							}
						}
					}

					return true;
				}

				return false;
			}));

		InitIcon();

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

	if ((changedAttributes.find(s_IconAttribId) != changedAttributes.cend()) 
		|| (changedAttributes.find(s_IconLvalAttribId) != changedAttributes.cend())
		|| (changedAttributes.find(s_IconRvalAttribId) != changedAttributes.cend()))
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

//---------------------------------
// ElementWindowHandle::OnChildAdd
//
void ElementWindowHandle::OnChildAdd(Element* const child)
{
	if (child->IsClassSet(s_HitIgnoreClassName))
	{
		m_HitTestIgnoreElements.emplace_back(ToPtr(child));
	}
	
	if (child->IsClassSet(s_HitIgnoreChildrenClassName))
	{
		m_HitTestIgnoreChildren.emplace_back(ToPtr(child));
	}
}

//------------------------------------
// ElementWindowHandle::OnChildRemove
//
void ElementWindowHandle::OnChildRemove(Element* const child)
{
	auto const foundIt = std::find(m_HitTestIgnoreElements.begin(), m_HitTestIgnoreElements.end(), child);
	if (foundIt != m_HitTestIgnoreElements.cend())
	{
		core::RemoveSwap(m_HitTestIgnoreElements, foundIt);
	}

	auto const foundChildrenIt = std::find(m_HitTestIgnoreChildren.begin(), m_HitTestIgnoreChildren.end(), child);
	if (foundChildrenIt != m_HitTestIgnoreChildren.cend())
	{
		core::RemoveSwap(m_HitTestIgnoreChildren, foundChildrenIt);
	}
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
	Rml::ElementUtilities::PositionElement(m_ControlsArea.Get(), Rml::Vector2f(dim.x/*controlW -dim.x*/, 0), Rml::ElementUtilities::TOP_LEFT);
	Rml::ElementUtilities::FormatElement(m_ControlsArea.Get(), Rml::Vector2f(controlW, dim.y));


	Rml::ElementUtilities::PositionElement(m_MinButton.Get(), Rml::Vector2f(0, 0), Rml::ElementUtilities::TOP_LEFT);
	Rml::ElementUtilities::FormatElement(m_MinButton.Get(), Rml::Vector2f(dim.y, dim.y));

	Rml::ElementUtilities::PositionElement(m_MaxButton.Get(), Rml::Vector2f(dim.y, 0), Rml::ElementUtilities::TOP_LEFT);
	Rml::ElementUtilities::FormatElement(m_MaxButton.Get(), Rml::Vector2f(dim.y, dim.y));

	Rml::ElementUtilities::PositionElement(m_CloseButton.Get(), Rml::Vector2f(dim.y * 2, 0), Rml::ElementUtilities::TOP_LEFT);
	Rml::ElementUtilities::FormatElement(m_CloseButton.Get(), Rml::Vector2f(dim.y, dim.y));

	// make sure client area flows around the handle
	SetProperty(Rml::PropertyId::PaddingLeft, 
		Rml::Property((m_Icon != nullptr) ? m_Icon->GetBox().GetSize(Rml::Box::MARGIN).x : 0.f, Rml::Property::Unit::PX));
	SetProperty(Rml::PropertyId::PaddingRight, Rml::Property(m_ControlsArea->GetBox().GetSize(Rml::Box::MARGIN).x, Rml::Property::Unit::PX));
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
		attributes.emplace("width", s_Height - 1);
		attributes.emplace("height", s_Height - 1);
		attributes["src"] = iconId;

		// add classes to the icon
		std::string const iconClass = GetAttribute<std::string>(s_IconClassAttribId, "");
		if (!iconClass.empty())
		{
			attributes.emplace("class", iconClass);
		}

		// lvals and rvals for the icon allow attaching data expressions
		std::string const iconLVal = GetAttribute<std::string>(s_IconLvalAttribId, "");
		if (!iconLVal.empty())
		{
			std::string const iconRVal = GetAttribute<std::string>(s_IconRvalAttribId, "");
			attributes.emplace(iconLVal, iconRVal);
		}
		else
		{
			ET_ASSERT(GetAttribute<std::string>(s_IconRvalAttribId, "").empty(), "icon rval found but no lval");
		}

		m_Icon = ToPtr(AppendChild(Rml::Factory::InstanceElement(this, "svg", "icon", attributes), false));

		DirtyLayout();
	}
}

//-----------------------------------------
// ElementWindowHandle::OnMaximizedChanged
//
void ElementWindowHandle::OnMaximizedChanged(bool const maximized)
{
	SetPseudoClass("maximized", maximized);

	m_MaxButton->SetAttribute("src", maximized ? "icons/restore.svg" : "icons/maximize.svg");
	// #todo: there is a bug in the RML ui implementation that means that the texture only updates if the image resizes
	m_MaxButton->SetAttribute("height", s_Height - (maximized ? 0 : 1)); 
}


} // namespace app
} // namespace et

