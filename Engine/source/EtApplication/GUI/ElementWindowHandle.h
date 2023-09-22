#pragma once
#include <RmlUi/Core/Element.h>
#include <RmlUi/Core/EventListener.h>

#include <EtApplication/GuiWindow.h>


namespace et {
namespace app {


//---------------------
// ElementWindowHandle
//
// Element that allows dragging a GuiWindow
//
class ElementWindowHandle : public Rml::Element
{
	// definitions
	//-------------

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

	static float const s_Height;
public:
	static std::string const s_IconAttribId;

	// construct destruct
	//--------------------
	RMLUI_RTTI_DefineWithParent(ElementWindowHandle, Rml::Element)

	ElementWindowHandle(Rml::String const& tag);
	virtual ~ElementWindowHandle();

	void LazyInit();

	// interface
	//-----------
protected:
	void OnAttributeChange(Rml::ElementAttributes const& changedAttributes) override;
	void ProcessDefaultAction(Rml::Event& evnt) override;
	void OnLayout() override;
	void OnResize() override;

	// utility
	//---------
private:
	void OnButtonEvent(Rml::Event& evnt);
	void FormatChildren();
	void InitIcon();

	void OnMaximizedChanged(bool const maximized);


	// Data
	///////

	Ptr<GuiWindow> m_Window;

	Ptr<Rml::Element> m_ControlsArea;

	Ptr<Rml::Element> m_MinButton;
	Ptr<Rml::Element> m_MaxButton;
	Ptr<Rml::Element> m_CloseButton;

	Ptr<Rml::Element> m_Icon;

	GuiWindow::T_EventCallbackId m_WindowCallbackId = GuiWindow::INVALID_CALLBACK;
	WindowListener m_Listener;

	bool m_IsInitialized = false;
};


} // namespace app
} // namespace et
