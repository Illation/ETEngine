#pragma once
#include <rttr/type>


// forward
namespace Gtk {
	class Frame;
}


namespace et {
namespace edit {


class EditorSplitNode;
class EditorBase;


//---------------------------------
// EditorNode
//
// Abstract node that can contain a hierachy of subdivided tool views
//
class EditorNode
{
	// definitions
	//-------------
	RTTR_ENABLE()
	friend class EditorNodeHierachy;

	// construct destruct
	//--------------------
public:
	virtual ~EditorNode() = default;

	// functionality
	//----------------
	void Init(EditorBase* const editor, Gtk::Frame* const attachment, WeakPtr<EditorSplitNode> const& parent, WeakPtr<EditorNode> const& self);
	Gtk::Frame* UnlinkAttachment();

	Gtk::Frame* GetAttachment() const { return m_Attachment; }
	WeakPtr<EditorSplitNode> const& GetParent() const { return m_Parent; }

	// accessors
	//-----------
	bool ContainsPointer() const;

	// interface
	//----------------
	virtual bool IsLeaf() const = 0;
protected:
	virtual void InitInternal(EditorBase* const editor) = 0;

	// Data
	///////

	Gtk::Frame* m_Attachment = nullptr;
	WeakPtr<EditorSplitNode> m_Parent;
	WeakPtr<EditorNode> m_ThisWeak; // #todo: temp - intrusive refcount can fix this
};


} // namespace edit
} // namespace et
