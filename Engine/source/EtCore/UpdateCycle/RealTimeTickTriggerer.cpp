#include "stdafx.h"

#include "RealTimeTickTriggerer.h"

#include "TickManager.h"


namespace et {
namespace core {


//===================================
// realtime tick triggerer interface
//===================================


//---------------------------------
// I_RealTimeTickTriggerer::~I_RealTimeTickTriggerer
//
// Make sure we are removed from tick manager on destruction
//
I_RealTimeTickTriggerer::~I_RealTimeTickTriggerer()
{
	UnRegisterAsTriggerer();
}

//---------------------------------
// I_RealTimeTickTriggerer::TriggerTick
//
// Try triggering a tick in the tick manager. this will succeed if this class has already triggered the tick in the last cycle
//
void I_RealTimeTickTriggerer::TriggerTick()
{
	TickManager::GetInstance()->TriggerRealTime(this);
}

//---------------------------------
// I_RealTimeTickTriggerer::RegisterAsTriggerer
//
// Register ourselves to the tick manager. we should only be registered if we are calling the trigger tick function every cycle
//
void I_RealTimeTickTriggerer::RegisterAsTriggerer()
{
	TickManager::GetInstance()->RegisterRealTimeTriggerer(this);
}

//---------------------------------
// I_RealTimeTickTriggerer::UnRegisterAsTriggerer
//
// Unregister ourselves from the tick manager. Should be called if we stop triggering ticks in real time
//
void I_RealTimeTickTriggerer::UnRegisterAsTriggerer()
{
	TickManager::GetInstance()->UnregisterRealTimeTriggerer(this);
}


} // namespace core
} // namespace et
