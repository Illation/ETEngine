#pragma once
#include "EditorNode.h"

#include <gdk/gdk.h>


//---------------------------------
// EditorBase
//
// Abstract editor that could manipulate some sort of resource. The init function should attach the editors UI to the parent frame 
//
class EditorBase
{
public:
	virtual ~EditorBase() = default;

	// interface
	//------------
protected:
	virtual void InitInternal() = 0;
public:
	virtual std::string const& GetName() const = 0;
	virtual std::string const& GetLayoutName() const = 0;
	virtual std::vector<E_EditorTool> const& GetSupportedTools() const = 0;

	// accessors
	//-----------
	Gtk::Frame* GetRoot();

	// functionality
	//------------------
	void Init(Gtk::Frame* const parent);
	void SaveLayout();

	void OnAllocationAvailable();

	virtual bool OnKeyEvent(bool const pressed, GdkEventKey* const evnt);
protected:

	// Data
	///////

	EditorNodeHierachy m_NodeHierachy;

	bool m_HasInitialSize = false;
};

