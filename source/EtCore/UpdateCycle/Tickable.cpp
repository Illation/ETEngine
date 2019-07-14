#include "stdafx.h"

#include "Tickable.h"

#include "TickManager.h"


//====================
// Tickable interface
//====================


//---------------------------------
// I_Tickable::I_Tickable
//
// Construct an I_Tickable with a priority and register it to the tick manager
//
I_Tickable::I_Tickable(uint32 const priority)
{
	TickManager::GetInstance()->RegisterTickable(this, priority);
}

//---------------------------------
// I_Tickable::~I_Tickable
//
// Unregister this from tick manager on destruction
//
I_Tickable::~I_Tickable()
{
	TickManager::GetInstance()->UnregisterTickable(this);
}

