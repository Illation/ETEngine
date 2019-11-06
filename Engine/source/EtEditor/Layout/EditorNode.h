#pragma once
#include <rttr/type>


// forward
namespace Gtk {
	class Frame;
}

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
	void Init(EditorBase* const editor, Gtk::Frame* const attachment, EditorSplitNode* const parent);
	Gtk::Frame* UnlinkAttachment();

	Gtk::Frame* GetAttachment() const { return m_Attachment; }
	EditorSplitNode* GetParent() const { return m_Parent; }

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
	EditorSplitNode* m_Parent = nullptr;
};

