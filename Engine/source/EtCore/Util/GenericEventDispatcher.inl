#pragma once
#include <algorithm>


namespace et {
namespace core {


//=====================================
// Generic Event Dispatcher :: Listener
//=====================================


//-------------------------------------------
// GenericEventDispatcher::Listener::c-tor
//
// Register a listener to all events matching the flags
//
template <typename TFlagType, class TEventData>
GenericEventDispatcher<TFlagType, TEventData>::Listener::Listener(TFlagType const eventFlags, T_CallbackFn& func)
	: flags(eventFlags)
{
	callback = func;
}


//===========================
// Generic Event Dispatcher
//===========================


//---------------------------------
// GenericEventDispatcher::Register
//
// Register a listener to all events matching the flags
//
template <typename TFlagType, class TEventData>
typename GenericEventDispatcher<TFlagType, TEventData>::T_CallbackId GenericEventDispatcher<TFlagType, TEventData>::Register(TFlagType const flags, 
	T_CallbackFn& callback)
{
	return m_Listeners.insert(Listener(flags, callback)).second;
}

//----------------------------------
// GenericEventDispatcher::Unregister
//
// Stop sending a listener events - sets the callback ID to invalid
//
template <typename TFlagType, class TEventData>
void GenericEventDispatcher<TFlagType, TEventData>::Unregister(T_CallbackId& callbackId)
{
	ET_ASSERT(callbackId != INVALID_ID);

	m_Listeners.erase(callbackId);
	callbackId = INVALID_ID;
}

//---------------------------------
// GenericEventDispatcher::Notify
//
// Notify all listeners registered to this event of the change immediately - deletes the event data after all listeners have been notified
//
template <typename TFlagType, class TEventData>
void GenericEventDispatcher<TFlagType, TEventData>::Notify(TFlagType const eventType, TEventData const* const eventData)
{
	// iterate over a copy of the lsit in order to make listeners unregistering themselves during their callback safe
	slot_map<Listener> notificationListeners = m_Listeners;

	for (Listener& listener : notificationListeners) 
	{
		// check if the listener is listening for our event type
		if (listener.flags & eventType)
		{
			listener.callback(eventType, eventData);
		}
	}

	delete eventData;
}


} // namespace core
} // namespace et
