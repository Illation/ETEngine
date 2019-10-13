#pragma once
#include <EtEditor/UI/EditorTool.h>


// forward
namespace Gtk {
	class Frame;
	class Paned;
}


//---------------------------------
// EditorBase
//
// Abstract editor that could manipulate some sort of resource. The init function should attach the editors UI to the parent frame 
//
class EditorBase
{
public:
	virtual ~EditorBase() = default;

	// utility
	//----------
	static Gtk::Frame* CreateInnerFrame(Gtk::Paned* const split, bool const isFirst);

	// interface
	//------------
	virtual void Init(Gtk::Frame* const parent) = 0;
	virtual std::string const& GetName() const = 0;

	// functionality
	//------------------
protected:
	void CreateTool(E_EditorTool const toolType, Gtk::Frame* const parent);

	// Data
	///////
	
	std::vector<std::unique_ptr<I_EditorTool>> m_Tools;
};

