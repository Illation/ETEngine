#include "stdafx.h"
#include "ToolNodeFeedback.h"

#include <cairomm/context.h>


namespace et {
namespace edit {


//====================
// Tool Node Feedback
//====================


double const ToolNodeFeedback::s_LineWidth = 2.0;


//---------------------------------
// ToolNodeFeedback::on_draw
//
bool ToolNodeFeedback::on_draw(Cairo::RefPtr<Cairo::Context> const& cr)
{
	Gtk::Allocation const allocation = get_allocation();
	double const width = static_cast<double>(allocation.get_width());
	double const height = static_cast<double>(allocation.get_height());

	cr->push_group();
	cr->set_source_rgba(0.0, 0.0, 0.0, 0.5);
	cr->rectangle(0.0, 0.0, width, height);
	cr->fill();

	// draw a split line
	cr->set_source_rgba(1.0, 1.0, 1.0, 1.0);
	cr->set_line_width(s_LineWidth);
	if (m_State == E_State::HSplit)
	{
		cr->move_to(static_cast<double>(m_SplitPos), 0.0);
		cr->line_to(static_cast<double>(m_SplitPos), height);
		cr->stroke();
	}
	else if (m_State == E_State::VSplit)
	{
		cr->move_to(0.0, static_cast<double>(m_SplitPos));
		cr->line_to(width, static_cast<double>(m_SplitPos));
		cr->stroke();
	}
	else if (m_State == E_State::Collapse)
	{
		// generate the path for a filled arrow
		cr->move_to(width * 0.5, height * 0.5);
		switch (m_Border)
		{
		case E_Border::Left:
			cr->line_to(width * 0.25, height * 0.25);
			cr->line_to(width * 0.25, height * 0.375);
			cr->line_to(width * 0.0, height * 0.375);
			cr->line_to(width * 0.0, height * 0.625);
			cr->line_to(width * 0.25, height * 0.625);
			cr->line_to(width * 0.25, height * 0.75);
			break;

		case E_Border::Right:
			cr->line_to(width * 0.75, height * 0.25);
			cr->line_to(width * 0.75, height * 0.375);
			cr->line_to(width * 1.0, height * 0.375);
			cr->line_to(width * 1.0, height * 0.625);
			cr->line_to(width * 0.75, height * 0.625);
			cr->line_to(width * 0.75, height * 0.75);
			break;

		case E_Border::Top:
			cr->line_to(width * 0.25, height * 0.25);
			cr->line_to(width * 0.375, height * 0.25);
			cr->line_to(width * 0.375, height * 0.0);
			cr->line_to(width * 0.625, height * 0.0);
			cr->line_to(width * 0.625, height * 0.25);
			cr->line_to(width* 0.75, height * 0.25);
			break;

		case E_Border::Bottom:
			cr->line_to(width * 0.25, height * 0.75);
			cr->line_to(width * 0.375, height * 0.75);
			cr->line_to(width * 0.375, height * 1.0);
			cr->line_to(width * 0.625, height * 1.0);
			cr->line_to(width * 0.625, height * 0.75);
			cr->line_to(width * 0.75, height * 0.75);
			break;
		}
		cr->close_path();

		// draw the arrow, subtracting from the surface
		Cairo::Operator const opDefault = cr->get_operator();

		cr->set_operator(Cairo::Operator::OPERATOR_DEST_OUT);
		cr->set_source_rgba(1.0, 1.0, 1.0, 0.5);
		cr->fill();

		cr->set_operator(opDefault);
	}

	cr->pop_group_to_source();
	cr->rectangle(0.0, 0.0, width, height);
	cr->fill();

	return true;
}

//---------------------------------
// ToolNodeFeedback::ForceRedraw
//
bool ToolNodeFeedback::ForceRedraw()
{
	auto win = get_window();
	if (win)
	{
		Gdk::Rectangle r(0, 0, get_allocation().get_width(), get_allocation().get_height());
		win->invalidate_rect(r, false);
	}

	return true;
}


} // namespace edit
} // namespace et
