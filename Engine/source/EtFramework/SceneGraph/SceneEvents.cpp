#include "stdafx.h"
#include "SceneEvents.h"

#include <algorithm>


//====================================
// Scene Event Dispatcher :: Listener
//====================================


//-------------------------------------------
// SceneEventDispatcher::Listener::c-tor
//
// Register a listener to all events matching the flags
//
SceneEventDispatcher::Listener::Listener(T_SceneEventCallbackId const callbackId, T_SceneEventFlags const eventFlags, T_SceneEventCallback& func)
	: id(callbackId)
	, flags(eventFlags)
{ 
	callback = func;
}


//===========================
// Scene Event Dispatcher
//===========================


// statics
T_SceneEventCallbackId const SceneEventDispatcher::s_InvalidCallbackId = 0u;


//---------------------------------
// SceneEventDispatcher::c-tor
//
// default constructor makes sure we start returning a valid callback ID
//
SceneEventDispatcher::SceneEventDispatcher()
	: m_NextCallbackId(s_InvalidCallbackId + 1u)
{ }

//---------------------------------
// SceneEventDispatcher::Register
//
// Register a listener to all events matching the flags
//
T_SceneEventCallbackId SceneEventDispatcher::Register(T_SceneEventFlags const flags, T_SceneEventCallback& callback)
{
	m_Listeners.push_back(Listener(m_NextCallbackId, flags, callback));
	return m_NextCallbackId++;
}

//----------------------------------
// SceneEventDispatcher::Unregister
//
// Stop sending a listener events - sets the callback ID to invalid
//
void SceneEventDispatcher::Unregister(T_SceneEventCallbackId& callbackId)
{
	// try finding the listener
	auto listenerIt = std::find_if(m_Listeners.begin(), m_Listeners.end(), [callbackId](Listener const& listener)
		{
			return listener.id == callbackId;
		});

	// it should have been found
	if (listenerIt == m_Listeners.cend())
	{
		LOG(FS("SceneEventDispatcher::Unregister > No listener found for callback ID '%u'", callbackId), LogLevel::Warning);

		callbackId = s_InvalidCallbackId;
		return;
	}

	// ensure invalidation of the callback ID for the listener
	callbackId = s_InvalidCallbackId;

	// swap and remove - the order of the listener list doesn't matter
	if (m_Listeners.size() > 1u)
	{
		std::iter_swap(listenerIt, std::prev(m_Listeners.end()));
		m_Listeners.pop_back();
	}
	else
	{
		m_Listeners.clear();
	}
}

//---------------------------------
// SceneEventDispatcher::Notify
//
// Notify all listeners registered to this event of the change immediately - deletes the event data after all listeners have been notified
//
void SceneEventDispatcher::Notify(SceneEventData const* const eventData)
{
	ET_ASSERT(eventData != nullptr);

	// Copy ensures listeners can unregister themselves during callback
	std::vector<Listener> listenersCopy = m_Listeners;

	// consider each listener
	for (Listener const& listener : listenersCopy)
	{
		// check if it is listening for our event type
		if (listener.flags & eventData->eventType)
		{
			listener.callback(eventData);
		}
	}

	delete eventData;
}
