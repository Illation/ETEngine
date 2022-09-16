#pragma once
#include <RmlUi/Core/Element.h>
#include <RmlUi/Core/EventListener.h>


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
	class WindowListener : public Rml::EventListener
	{
	public:
		virtual ~WindowListener() = default;

		void SetHandle(Ptr<ElementWindowHandle> const handle) { m_WindowHandle = handle; }

		void ProcessEvent(Rml::Event& evnt) override;

	private:
		Ptr<ElementWindowHandle> m_WindowHandle;
	};

	friend class WindowListener;

	// construct destruct
	//--------------------
public:
	RMLUI_RTTI_DefineWithParent(ElementWindowHandle, Rml::Element)

	ElementWindowHandle(Rml::String const& tag);
	virtual ~ElementWindowHandle() = default;

	void LazyInit();

	// interface
	//-----------
protected:
	void ProcessDefaultAction(Rml::Event& evnt) override;
	void OnResize() override;

	// utility
	//---------
private:
	void OnButtonEvent(Rml::Event& evnt);
	void FormatChildren();


	// Data
	///////

	Ptr<GuiWindow> m_Window;

	Ptr<Rml::Element> m_ControlsArea;

	Ptr<Rml::Element> m_MinButton;
	Ptr<Rml::Element> m_MaxButton;
	Ptr<Rml::Element> m_CloseButton;

	WindowListener m_Listener;
};


} // namespace app
} // namespace et
