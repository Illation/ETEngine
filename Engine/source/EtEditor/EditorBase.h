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

	// functionality
	//------------------
	void Init(Gtk::Frame* const parent);
	void SaveLayout();

	virtual bool OnKeyEvent(bool const pressed, GdkEventKey* const evnt);
protected:

	// Data
	///////

	EditorNodeHierachy m_NodeHierachy;
};

