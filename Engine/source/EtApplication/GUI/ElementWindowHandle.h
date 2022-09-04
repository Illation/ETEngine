#pragma once
#include <RmlUi/Core/Element.h>


namespace et {
namespace app {


// fwd
class GuiWindow;


//---------------------
// ElementWindowHandle
//
// Element that allows dragging a GuiWindow
//
class ElementWindowHandle : public Rml::Element
{
	// construct destruct
	//--------------------
public:
	RMLUI_RTTI_DefineWithParent(ElementWindowHandle, Rml::Element)

	ElementWindowHandle(Rml::String const& tag);
	virtual ~ElementWindowHandle() = default;

	// interface
	//-----------
protected:
	void ProcessDefaultAction(Rml::Event& evnt);


	// Data
	///////

private:
	Ptr<GuiWindow> m_Window;
};


} // namespace app
} // namespace et
