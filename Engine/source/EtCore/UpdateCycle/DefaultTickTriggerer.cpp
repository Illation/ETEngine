#include "stdafx.h"

#include "DefaultTickTriggerer.h"

#include "TickManager.h"


namespace et {
namespace core {


//===================================
// default tick triggerer interface
//===================================


//---------------------------------
// I_DefaultTickTriggerer::~I_DefaultTickTriggerer
//
// Make sure we are removed from tick manager on destruction
//
I_DefaultTickTriggerer::~I_DefaultTickTriggerer()
{
	UnRegisterAsTriggerer();
}

//---------------------------------
// I_DefaultTickTriggerer::TriggerTick
//
// Try triggering a tick in the tick manager. this will succeed if this class has already triggered the tick in the last cycle, if there are no realtime
//  - triggerers
//
void I_DefaultTickTriggerer::TriggerTick()
{
	TickManager::GetInstance()->TriggerDefault(this);
}

//---------------------------------
// I_DefaultTickTriggerer::RegisterAsTriggerer
//
// Register ourselves to the tick manager. 
//
void I_DefaultTickTriggerer::RegisterAsTriggerer()
{
	TickManager::GetInstance()->RegisterDefaultTriggerer(this);
}

//---------------------------------
// I_DefaultTickTriggerer::UnRegisterAsTriggerer
//
// Unregister ourselves from the tick manager. 
//
void I_DefaultTickTriggerer::UnRegisterAsTriggerer()
{
	TickManager::GetInstance()->UnregisterDefaultTriggerer(this);
}


} // namespace core
} // namespace et
