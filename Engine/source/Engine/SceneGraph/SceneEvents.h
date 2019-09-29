#pragma once

#include <functional>


//---------------------------
// E_SceneEvent
//
// List of events systems can listen for
//
typedef uint8 T_SceneEventFlags;
enum E_SceneEvent : T_SceneEventFlags
{
	Invalid = 0,

	Initialized = 1 << 0,

	All = 0xFF
};

//---------------------------
// SceneEventData
//
// Base scene event data contains only event type, but can be derived from to provide additional data
//
struct SceneEventData
{
public:
	SceneEventData(E_SceneEvent const evnt) : eventType(evnt) {}
	virtual ~SceneEventData() = default;

	E_SceneEvent eventType;
};

typedef size_t T_SceneEventCallbackId;
typedef std::function<void(SceneEventData const* const)> T_SceneEventCallback;

//---------------------------
// SceneEventDispatcher
//
// Manages listeners and sends them notifications
//
class SceneEventDispatcher final
{
private:

	// definitions
	//----------------

	//---------------------------
	// E_SceneEvent::Listener
	//
	// All the information needed to manage a callback
	//
	struct Listener
	{
		Listener(T_SceneEventCallbackId const callbackId, T_SceneEventFlags const eventFlags, T_SceneEventCallback& func);

		T_SceneEventCallbackId id;
		T_SceneEventFlags flags;
		T_SceneEventCallback callback;
	};

public:
	static T_SceneEventCallbackId const s_InvalidCallbackId;

	// construct destruct
	//---------------------
	SceneEventDispatcher();
	~SceneEventDispatcher() = default;

	// functionality
	//---------------
	T_SceneEventCallbackId Register(T_SceneEventFlags const flags, T_SceneEventCallback& callback);
	void Unregister(T_SceneEventCallbackId& callbackId);

	void Notify(SceneEventData const* const eventData); // maybe should be private to only entities and AbstractScene

private:

	// Data
	///////

	std::vector<Listener> m_Listeners;
	T_SceneEventCallbackId m_NextCallbackId;
};

