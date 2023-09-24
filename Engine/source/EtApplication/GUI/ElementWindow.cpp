#include "stdafx.h"
#include "ElementWindow.h"

#include <RmlUi/Core/Context.h>
#include <RmlUi/Core/Factory.h>

#include <EtApplication/GuiApplication.h>
#include <EtApplication/GuiWindow.h>

#include <EtGUI/Context/RmlUtil.h>


namespace et {
namespace app {


//================
// Window Element
//================


// static
std::string const ElementWindow::s_CustomWindowHandleId("handle-id");


//---------------------------------------
// ElementWindow::OnFocusChangeRecursive
//
void ElementWindow::OnFocusChangeRecursive(Rml::Element* const el, bool const focused)
{
	el->SetPseudoClass("window_inactive", !focused);

	for (int childIdx = 0u; childIdx < el->GetNumChildren(true); ++childIdx)
	{
		OnFocusChangeRecursive(el->GetChild(childIdx), focused);
	}
}

//----------------------------
// ElementWindow::c-tor
//
ElementWindow::ElementWindow(Rml::String const& tag)
	: Rml::Element(tag)
{
	SetProperty(Rml::PropertyId::PointerEvents, Rml::Property(Rml::Style::PointerEvents::Auto));
}

//----------------------------
// ElementWindow::d-tor
//
ElementWindow::~ElementWindow()
{
	if (m_WindowCallbackId != GuiWindow::INVALID_CALLBACK)
	{
		ET_ASSERT(m_Window != nullptr);
		m_Window->UnregisterCallback(m_WindowCallbackId);
	}
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
		ET_ASSERT(reinterpret_cast<Rml::Element const*>(GetOwnerDocument()) == GetParentNode(), "window elements must be their owner documents first child");

		m_WindowCallbackId = m_Window->RegisterCallback(GuiWindow::GW_All,
			GuiWindow::T_EventCallback([this](GuiWindow::T_EventFlags const evnt, GuiWindow::EventData const* const)
				{
					switch (static_cast<GuiWindow::E_Event>(evnt))
					{
					case GuiWindow::GW_GainFocus:
						OnFocusChangeRecursive(this, true);
						break;
					case GuiWindow::GW_LooseFocus:
						OnFocusChangeRecursive(this, false);
						break;
					case GuiWindow::GW_Maximize:
						OnMaximizedChanged(true);
						break;
					case GuiWindow::GW_Restore:
						OnMaximizedChanged(false);
						break;
					}
				}));

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

		OnFocusChangeRecursive(this, m_Window->Focused());

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

//---------------------------------------
// ElementWindow::GetIntrinsicDimensions
//
bool ElementWindow::GetIntrinsicDimensions(Rml::Vector2f& dimensions, float& ratio)
{
	Rml::Vector2i const dim = GetContext()->GetDimensions(); // we assume here that windows are always the same size as the context
	dimensions = Rml::Vector2f(static_cast<float>(dim.x), static_cast<float>(dim.y));
	ratio = dimensions.x / dimensions.y;
	return true;
}

//------------------------------------
// ElementWindow::FormatChildren
//
void ElementWindow::FormatChildren()
{
}

//------------------------------------
// ElementWindow::OnMaximizedChanged
//
void ElementWindow::OnMaximizedChanged(bool const maximized)
{
	SetPseudoClass("maximized", maximized);
}


} // namespace app
} // namespace et

