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

	// construct destruct
	//--------------------
public:
	RMLUI_RTTI_DefineWithParent(ElementWindow, Rml::Element)

	ElementWindow(Rml::String const& tag);
	virtual ~ElementWindow() = default;

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


	// Data
	///////

	Ptr<GuiWindow> m_Window;
	Ptr<ElementWindowHandle> m_WindowHandleEl;

	bool m_IsInitialized = false;
};


} // namespace app
} // namespace et

