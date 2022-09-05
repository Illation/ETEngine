#include "stdafx.h"
#include "ElementWindowHandle.h"

#include <EtApplication/GuiApplication.h>
#include <EtApplication/GuiWindow.h>


namespace et {
namespace app {


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
}

//-------------------------------------------
// ElementWindowHandle::ProcessDefaultAction
//
void ElementWindowHandle::ProcessDefaultAction(Rml::Event& evnt)
{
	Rml::Element::ProcessDefaultAction(evnt);

	if (evnt.GetTargetElement() == this)
	{
		if (m_Window == nullptr)
		{
			m_Window = GuiApplication::Instance()->GetWindow(GetContext());
			ET_ASSERT(m_Window != nullptr);
		}

		if (evnt == Rml::EventId::Dragstart)
		{
			m_Window->StartDrag();
			ET_LOG_I(ET_CTX_APP, "Window Drag Start");
		}
	}
}


} // namespace app
} // namespace et

