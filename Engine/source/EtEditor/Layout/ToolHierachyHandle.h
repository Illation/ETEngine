#pragma once
#include <gdk/gdk.h>
#include <glibmm/refptr.h>

#include <rttr/registration_friend.h>


// forward
namespace Gtk {
	class EventBox;
	class Image;
}
namespace Gdk {
	class Cursor;
}


namespace et {
namespace edit {


class EditorToolNode;


//---------------------------------
// ToolHierachyHandle
//
// Handle added to tool leaf nodes to allow splitting and merging of tools
//
class ToolHierachyHandle final
{
	// definitions
	//-------------
	RTTR_REGISTRATION_FRIEND

	static float const s_SplitThreshold;

	enum class E_DragState
	{
		None,
		Start,
		Abort,

		CollapseNeighbour,
		CollapseOwner,
		CollapseAbort,

		VSplit,
		VSplitAbort,

		HSplit,
		HSplitAbort
	};

	// construct destruct
	//--------------------
public:
	~ToolHierachyHandle() = default;

	// functionality
	//---------------
	void Init(EditorToolNode* const owner, bool right, bool top);
private:
	void SetCornerImage();

	void ProcessDrag(GdkEventMotion* const motion);
	void ActionDragResult();

	// Data
	///////

	EditorToolNode* m_Owner = nullptr;
	EditorToolNode* m_Neighbour = nullptr;

	bool m_IsRightAligned = false;
	bool m_IsTopAligned = false;

	Gtk::EventBox* m_EventBox = nullptr;
	Gtk::Image* m_Image = nullptr;

	Glib::RefPtr<Gdk::Cursor> m_CursorCross;

	E_DragState m_DragState = E_DragState::None;
	ivec2 m_Position; // in start state represents the initial position, otherwise the position within the owning frame in case of a split
};


} // namespace edit
} // namespace et
