#pragma once



//---------------------------------
// I_Tickable
//
// Interface for things that should update regularly
//
class I_Tickable
{
protected:
	I_Tickable(uint32 const priority);
	virtual ~I_Tickable();

	I_Tickable& operator=(const I_Tickable&) { return *this; } 

public:
	virtual void OnTick() {}
};

