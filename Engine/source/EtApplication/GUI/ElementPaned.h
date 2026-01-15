#pragma once
#include <RmlUi/Core/Element.h>
#include <RmlUi/Core/EventListener.h>

#include <EtGUI/Context/Context.h>


namespace et {
namespace app {


//---------------------
// ElementPaned
//
// Element that creates a draggable split between two child elements
//
class ElementPaned : public Rml::Element
{
	// definitions
	//-------------
	class PanedListener : public Rml::EventListener, public gui::Context::I_MouseUpListener
	{
	public:
		virtual ~PanedListener() = default;

		void SetHandle(Ptr<ElementPaned> const handle) { m_PanedHandle = handle; }

		void ProcessEvent(Rml::Event& evnt) override;

		bool OnMouseUp() override;

	private:
		Ptr<ElementPaned> m_PanedHandle;
	};

	friend class PanedListener;

public:
	static std::string const s_OrientationAttribId;
	static std::string const s_PositionAttribId;

	static std::string const s_SeparatorClassName;
	static float const s_SeparatorSize;

	enum class E_Orientation : uint8
	{
		Horizontal = 0u,
		Vertical
	};

	// construct destruct
	//--------------------
	RMLUI_RTTI_DefineWithParent(ElementPaned, Rml::Element)

	ElementPaned(Rml::String const& tag);
	virtual ~ElementPaned();

	ElementPaned(ElementPaned const& other) = delete;
	ElementPaned& operator=(ElementPaned const& other) = delete;

	ElementPaned(ElementPaned&& moving) = delete;
	ElementPaned& operator=(ElementPaned&& moving) = delete;

	// interface
	//-----------
protected:
	void OnAttributeChange(Rml::ElementAttributes const& changedAttributes) override;
	void OnLayout() override;
	void OnResize() override;

	void OnChildAdd(Element* const child) override;
	void OnChildRemove(Element* const child) override;

	void OnUpdate() override;

	// utility
	//---------
private:
	void OnButtonEvent(Rml::Event& evnt);
	void StopDrag();
	void FormatChildren();

	// Data
	///////

	PanedListener m_Listener;

	Ptr<Rml::Element> m_Child1;
	Ptr<Rml::Element> m_Separator; // generated
	Ptr<Rml::Element> m_Child2;

	E_Orientation m_Orientation = E_Orientation::Horizontal;
	float m_Position = 0.5f;
	bool m_Dragging = false;
};


} // namespace app
} // namespace et


