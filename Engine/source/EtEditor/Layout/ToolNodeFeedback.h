#pragma once
#include <gtkmm/drawingarea.h>
#include <glibmm/refptr.h>


//---------------------------------
// ToolNodeFeedback
//
// Surface to draw into a tools overlat when collapsing or splitting the tool
//
class ToolNodeFeedback final : public Gtk::DrawingArea
{
	// definitions
	//-------------

	static double const s_LineWidth;

public:
	enum class E_State : uint8
	{
		Inactive,
		Collapse,
		HSplit,
		VSplit
	};

	enum class E_Border : uint8
	{
		Left,
		Right,
		Top,
		Bottom
	};

	// construct destruct
	//--------------------
	ToolNodeFeedback() = default;
	~ToolNodeFeedback() = default;

	// drawingArea interface
	//-----------------------
protected:
	bool on_draw(Cairo::RefPtr<Cairo::Context> const& cr) override;

	// accessors
	//---------------
public:
	E_State GetState() const { return m_State; }
	int32 GetSplitPos() const { return m_SplitPos; }

	// functionality
	//---------------
	void SetState(E_State const state) { m_State = state; }
	void SetBorder(E_Border const border) { m_Border = border; }
	void SetSplitPosition(int32 const splitPos) { m_SplitPos = splitPos; }
	bool ForceRedraw();

	// Data
	///////

private:
	E_State m_State = E_State::Inactive;
	E_Border m_Border = E_Border::Left;
	int32 m_SplitPos = 0;
};

