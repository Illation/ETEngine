#include "stdafx.h"
#include "ElementPaned.h"

#include <RmlUi/Core/Factory.h>
#include <RmlUi/Core/ElementUtilities.h>
#include <RmlUi/Core/Context.h>

#include <EtGUI/Context/Context.h>
#include <EtGUI/Context/RmlUtil.h>


namespace et {
namespace app {


//===========================
// Paned Element :: Listener
//===========================


//------------------------------
// PanedListener::ProcessEvent
//
void ElementPaned::PanedListener::ProcessEvent(Rml::Event& evnt)
{
	ET_ASSERT(m_PanedHandle != nullptr);
	m_PanedHandle->OnButtonEvent(evnt);
}

//------------------------------
// PanedListener::ProcessEvent
//
bool ElementPaned::PanedListener::OnMouseUp()
{
	ET_ASSERT(m_PanedHandle != nullptr);
	m_PanedHandle->StopDrag();
	return true;
}


//================
// Paned Element
//================


std::string const ElementPaned::s_OrientationAttribId("orientation");
std::string const ElementPaned::s_PositionAttribId("position");

std::string const ElementPaned::s_SeparatorClassName("paned-separator");
float const ElementPaned::s_SeparatorSize = 6.f;


//---------------------
// ElementPaned::c-tor
//
ElementPaned::ElementPaned(Rml::String const& tag)
	: Rml::Element(tag)
{ 
	m_Listener.SetHandle(ToPtr(this));
}

//---------------------
// ElementPaned::d-tor
//
ElementPaned::~ElementPaned()
{
	StopDrag();
}

//---------------------------------
// ElementPaned::OnAttributeChange
//
void ElementPaned::OnAttributeChange(Rml::ElementAttributes const& changedAttributes)
{
	Rml::Element::OnAttributeChange(changedAttributes);

	// orientation
	Rml::ElementAttributes::const_iterator orientationIt = changedAttributes.find(s_OrientationAttribId);
	if (orientationIt != changedAttributes.cend())
	{
		ET_ASSERT(orientationIt->second.GetType() == Rml::Variant::STRING);
		switch (GetHash(orientationIt->second.Get<std::string>()))
		{
		case "horizontal"_hash:
			m_Orientation = E_Orientation::Horizontal;
			break;

		case "vertical"_hash:
			m_Orientation = E_Orientation::Vertical;
			break;

		default:
			ET_WARNING("Use of undefined orientation '%s' in paned element '%s'", orientationIt->second.Get<std::string>().c_str(), GetId().c_str());
			break;
		}

		DirtyLayout();
	}

	// position
	Rml::ElementAttributes::const_iterator positionIt = changedAttributes.find(s_PositionAttribId);
	if (positionIt != changedAttributes.cend())
	{
		float pos;
		if (positionIt->second.GetInto(pos))
		{
			ET_ASSERT(pos >= 0.f && pos <= 100.f, "expected paned el position to have a value between 0 and 100! (%s)", GetId().c_str());
			m_Position = math::Clamp01(pos / 100.f);
		}
		else
		{
			ET_WARNING("Failed to read position value on  paned element '%s'", GetId().c_str());
		}

		DirtyLayout();
	}
}

//------------------------
// ElementPaned::OnLayout
//
void ElementPaned::OnLayout()
{
	Rml::Element::OnLayout();
	FormatChildren();
}

//------------------------
// ElementPaned::OnResize
//
void ElementPaned::OnResize()
{
	Rml::Element::OnResize();
	FormatChildren();
}

//--------------------------
// ElementPaned::OnChildAdd
//
void ElementPaned::OnChildAdd(Element* const child)
{
	Rml::Element::OnChildAdd(child);

	if (!child->IsClassSet(s_SeparatorClassName) && (child != this) && (child->GetParentNode() == this))
	{
		if (m_Child1 == nullptr)
		{
			m_Child1 = ToPtr(child);
		}
		else if (m_Child2 == nullptr)
		{
			m_Child2 = ToPtr(child);
		}
		else
		{
			ET_WARNING("Child added to paned but there where already two elements: %s", child->GetId().c_str());
		}

		// Create separator element when needed
		if ((m_Child1 != nullptr) && (m_Child2 != nullptr) && (m_Separator == nullptr))
		{
			// Create separator
			{
				Rml::ElementPtr sepPtr = Rml::Factory::InstanceElement(this, "*", "div", Rml::XMLAttributes());
				sepPtr->SetClass(s_SeparatorClassName, true);
				m_Separator = ToPtr(InsertBefore(std::move(sepPtr), m_Child2.Get()));
			}

			m_Separator->SetProperty(Rml::PropertyId::ZIndex, Rml::Property(2, Rml::Property::Unit::NUMBER));
			m_Separator->SetProperty(Rml::PropertyId::Drag, Rml::Property(Rml::Style::Drag::Drag));
			m_Separator->AddEventListener(Rml::EventId::Mousedown, &m_Listener);
		}

		FormatChildren();
	}
}

//-----------------------------
// ElementPaned::OnChildRemove
//
void ElementPaned::OnChildRemove(Element* const child)
{
	Rml::Element::OnChildRemove(child);

	if (m_Child1 == child)
	{
		m_Child1 = nullptr;
	}
	else if (m_Child2 == child)
	{
		m_Child2 = nullptr;
	}

	if (((m_Child1 == nullptr) || (m_Child2 == nullptr)) && (m_Separator != nullptr))
	{
		Rml::Element* sep = m_Separator.Get();
		m_Separator = nullptr;
		RemoveChild(sep);
	}
}

//------------------------
// ElementPaned::OnUpdate
//
void ElementPaned::OnUpdate()
{
	Rml::Element::OnUpdate();

	if (m_Dragging)
	{
		gui::Context const* const guiCtx = gui::Context::Get(GetContext());
		ET_ASSERT(guiCtx != nullptr);

		Rml::Vector2f mousePos = gui::RmlUtil::ToRml(math::vecCast<float>(guiCtx->GetMousePos()));
		mousePos.x -= GetAbsoluteLeft();
		mousePos.y -= GetAbsoluteTop();

		Rml::Vector2f const dim = GetBox().GetSize(Rml::Box::CONTENT);
		Rml::Vector2f sepDim;
		if ((m_Separator != nullptr) && (m_Separator->GetDisplay() != Rml::Style::Display::None))
		{
			sepDim = m_Separator->GetBox().GetSize(Rml::Box::MARGIN);
		}

		if (m_Orientation == E_Orientation::Horizontal)
		{
			m_Position = math::Clamp(mousePos.x - sepDim.x, dim.x - sepDim.x, 0.f) / dim.x;
		}
		else
		{
			m_Position = math::Clamp(mousePos.y - sepDim.y, dim.y - sepDim.y, 0.f) / dim.y;
		}

		DirtyLayout();
	}
}

//-----------------------------
// ElementPaned::OnButtonEvent
//
void ElementPaned::OnButtonEvent(Rml::Event& evnt)
{
	if (evnt.GetTargetElement() == m_Separator)
	{
		if (evnt.GetId() == Rml::EventId::Mousedown)
		{
			m_Dragging = true;

			//AddEventListener(Rml::EventId::Mouseout, &m_Listener, false);

			gui::Context* const guiCtx = gui::Context::Get(GetContext());
			ET_ASSERT(guiCtx != nullptr);
			guiCtx->RegisterMouseUpListener(ToPtr(&m_Listener));

			evnt.StopPropagation();
		}
	}
	else if (evnt.GetTargetElement() == this)
	{
		if (evnt.GetId() == Rml::EventId::Mouseout)
		{
			StopDrag();
		}
	}
}

//--------------------------
// ElementPaned::StopDrag
//
void ElementPaned::StopDrag()
{
	if (m_Dragging)
	{
		m_Dragging = false;

		//RemoveEventListener(Rml::EventId::Mouseout, &m_Listener, false);

		gui::Context* const guiCtx = gui::Context::Get(GetContext());
		ET_ASSERT(guiCtx != nullptr);
		guiCtx->UnregisterMouseUpListener(ToPtr(&m_Listener));
	}
}

//------------------------------
// ElementPaned::FormatChildren
//
// Calculate child element layout
//
void ElementPaned::FormatChildren()
{
	Rml::Vector2f const dim = GetBox().GetSize(Rml::Box::CONTENT);

	if (((m_Child1 == nullptr) || (m_Child1->GetDisplay() == Rml::Style::Display::None))
		|| ((m_Child2 == nullptr) || (m_Child2->GetDisplay() == Rml::Style::Display::None))
		|| (m_Separator == nullptr))
	{
		if (m_Separator != nullptr)
		{
			m_Separator->SetProperty("display", "none");
		}

		// try having a single full size child
		Rml::Element* child = nullptr;
		if ((m_Child1 != nullptr) && (m_Child1->GetDisplay() != Rml::Style::Display::None))
		{
			child = m_Child1.Get();
		}
		else if ((m_Child2 != nullptr) && (m_Child2->GetDisplay() != Rml::Style::Display::None))
		{
			child = m_Child2.Get();
		}

		if (child != nullptr)
		{
			Rml::ElementUtilities::PositionElement(child, Rml::Vector2f(0, 0), Rml::ElementUtilities::TOP_LEFT);
			Rml::ElementUtilities::FormatElement(child, Rml::Vector2f(dim.x, dim.y));
		}
	}
	else
	{
		ET_ASSERT(m_Separator != nullptr);
		m_Separator->SetProperty("display", "inline");

		m_Child1->SetProperty( Rml::PropertyId::FlexBasis, Rml::Property(m_Position * 100.f, Rml::Property::PERCENT) );
	}
}


} // namespace app
} // namespace et
