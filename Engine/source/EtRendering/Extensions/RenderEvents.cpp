#include "stdafx.h"
#include "RenderEvents.h"

#include <algorithm>


namespace render {


//=====================================
// Render Event Dispatcher :: Listener
//=====================================


//-------------------------------------------
// RenderEventDispatcher::Listener::c-tor
//
// Register a listener to all events matching the flags
//
RenderEventDispatcher::Listener::Listener(T_RenderEventFlags const eventFlags, T_RenderEventCallback& func)
	: flags(eventFlags)
{
	callback = func;
}


//===========================
// Render Event Dispatcher
//===========================


//---------------------------------
// RenderEventDispatcher::Register
//
// Register a listener to all events matching the flags
//
T_RenderEventCallbackId RenderEventDispatcher::Register(T_RenderEventFlags const flags, T_RenderEventCallback& callback)
{
	return m_Listeners.insert(Listener(flags, callback)).second;
}

//----------------------------------
// RenderEventDispatcher::Unregister
//
// Stop sending a listener events - sets the callback ID to invalid
//
void RenderEventDispatcher::Unregister(T_RenderEventCallbackId& callbackId)
{
	m_Listeners.erase(callbackId);
	callbackId = core::INVALID_SLOT_ID;
}

//---------------------------------
// RenderEventDispatcher::Notify
//
// Notify all listeners registered to this event of the change immediately - deletes the event data after all listeners have been notified
//
void RenderEventDispatcher::Notify(RenderEventData const* const eventData)
{
	ET_ASSERT(eventData != nullptr);

	// consider each listener
	// we do not copy the listener list because we don't expect anything to unregister itself during a callback
	for (Listener const& listener : m_Listeners) 
	{
		// check if it is listening for our event type
		if (listener.flags & eventData->eventType)
		{
			listener.callback(eventData);
		}
	}

	delete eventData;
}


} // namespace render
