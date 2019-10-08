#pragma once


namespace Gtk {
	class Frame;
}


//---------------------------------
// I_Editor
//
// Interface for an editor that could manipulate some sort of resource. The init function should attach the editors UI to the parent frame 
//
class I_Editor
{
public:
	virtual ~I_Editor() = default;

	virtual void Init(Gtk::Frame* const parent) = 0;
	virtual std::string const& GetName() const = 0;
};

