#pragma once
#include <RmlUi/Core/Element.h>

#include "ElementWindowHandle.h"


namespace et {
namespace app {


// fwd
class GuiWindow;


//---------------------
// ElementWindow
//
// Toplevel window element for custom decorated windows
//
class ElementWindow : public Rml::Element
{
	static std::string const s_CustomWindowHandleId;

	// static functionality
	//----------------------
public:
	static void OnFocusChangeRecursive(Rml::Element* const el, bool const focused);

	// construct destruct
	//--------------------
	RMLUI_RTTI_DefineWithParent(ElementWindow, Rml::Element)

	ElementWindow(Rml::String const& tag);
	virtual ~ElementWindow();

	void LazyInit();

	// interface
	//-----------
protected:
	void OnAttributeChange(Rml::ElementAttributes const& changedAttributes) override;
	void ProcessDefaultAction(Rml::Event& evnt) override;
	void OnUpdate() override;
	void OnResize() override;
	virtual bool GetIntrinsicDimensions(Rml::Vector2f& dimensions, float& ratio) override;

	// utility
	//---------
private:
	void FormatChildren();

	void OnMaximizedChanged(bool const maximized);


	// Data
	///////

	Ptr<GuiWindow> m_Window;

	Ptr<ElementWindowHandle> m_WindowHandleEl;

	GuiWindow::T_EventCallbackId m_WindowCallbackId = GuiWindow::INVALID_CALLBACK;

	bool m_IsInitialized = false;
};


} // namespace app
} // namespace et

