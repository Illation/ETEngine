#include "stdafx.h"
#include "ElementWindow.h"

#include <RmlUi/Core/Factory.h>

#include <EtApplication/GuiApplication.h>
#include <EtApplication/GuiWindow.h>


namespace et {
namespace app {


//================
// Window Element
//================


// static
std::string const ElementWindow::s_CustomWindowHandleId;


//----------------------------
// ElementWindow::c-tor
//
ElementWindow::ElementWindow(Rml::String const& tag)
	: Rml::Element(tag)
{
	SetProperty(Rml::PropertyId::PointerEvents, Rml::Property(Rml::Style::PointerEvents::Auto));
}

//-------------------------------
// ElementWindow::LazyInit
//
void ElementWindow::LazyInit()
{
	if (!m_IsInitialized)
	{
		m_Window = GuiApplication::Instance()->GetWindow(GetContext());
		ET_ASSERT(m_Window != nullptr);

		std::string const windowHandleId = GetAttribute<std::string>(s_CustomWindowHandleId, "");
		if (!windowHandleId.empty())
		{
			m_WindowHandleEl = ToPtr(static_cast<ElementWindowHandle*>(GetElementById(windowHandleId)));
			ET_ASSERT(m_WindowHandleEl != nullptr);
		}
		else 
		{
			Rml::XMLAttributes attributes;
			attributes.emplace(ElementWindowHandle::s_IconAttribId, GetAttribute<std::string>(ElementWindowHandle::s_IconAttribId, ""));

			Rml::ElementPtr handleEl = Rml::Factory::InstanceElement(this, "window_handle", "window_handle", attributes);

			if (HasChildNodes())
			{
				m_WindowHandleEl = ToPtr(static_cast<ElementWindowHandle*>(InsertBefore(std::move(handleEl), GetFirstChild())));
			}
			else
			{
				m_WindowHandleEl = ToPtr(static_cast<ElementWindowHandle*>(AppendChild(std::move(handleEl))));
			}

			m_WindowHandleEl->SetInnerRML(m_Window->GetTitle());
		}

		m_IsInitialized = true;
	}
}

//-------------------------------------------
// ElementWindow::OnAttributeChange
//
void ElementWindow::OnAttributeChange(Rml::ElementAttributes const& changedAttributes)
{
	Rml::Element::OnAttributeChange(changedAttributes);

	if (changedAttributes.find(s_CustomWindowHandleId) != changedAttributes.cend() ||
		changedAttributes.find(ElementWindowHandle::s_IconAttribId) != changedAttributes.cend())
	{
		m_IsInitialized = false;
		m_WindowHandleEl = nullptr; // might have to properly remove it too
	}
}

//-------------------------------------------
// ElementWindow::ProcessDefaultAction
//
void ElementWindow::ProcessDefaultAction(Rml::Event& evnt)
{
	Rml::Element::ProcessDefaultAction(evnt);

	if (evnt.GetTargetElement() == this)
	{
		LazyInit();
	}
}

//-------------------------------
// ElementWindow::OnUpdate
//
void ElementWindow::OnUpdate()
{
	Rml::Element::OnUpdate();
	LazyInit();
}

//-------------------------------
// ElementWindow::OnResize
//
void ElementWindow::OnResize()
{
	Rml::Element::OnResize();
	FormatChildren();
}

//------------------------------------
// ElementWindow::FormatChildren
//
void ElementWindow::FormatChildren()
{
}


} // namespace app
} // namespace et



