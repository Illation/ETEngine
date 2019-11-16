#pragma once
#include <EtCore/Containers/slot_map.h>


class I_ViewportRenderer;


namespace render {
	

//---------------------------
// E_RenderEvent
//
// List of events during the draw process
//
typedef uint8 T_RenderEventFlags;
enum E_RenderEvent : T_RenderEventFlags
{
	Invalid = 0,

	RenderDeferred	= 1 << 0,
	RenderLights	= 2 << 0,
	RenderForward	= 3 << 0,
	RenderOutlines	= 4 << 0,

	All = 0xFF
};


//---------------------------
// RenderEventData
//
// Base scene event data contains only event type and source renderer, but can be derived from to provide additional data
//
struct RenderEventData
{
public:
	RenderEventData(E_RenderEvent const evnt, I_ViewportRenderer const* const r, T_FbLoc const fb) : eventType(evnt), renderer(r), targetFb(fb) {}
	virtual ~RenderEventData() = default;

	E_RenderEvent eventType;
	I_ViewportRenderer const* renderer;
	T_FbLoc targetFb;
};


typedef core::T_SlotId T_RenderEventCallbackId;
typedef std::function<void(RenderEventData const* const)> T_RenderEventCallback;


//---------------------------
// RenderEventDispatcher
//
// Manages listeners and sends them notifications
//
class RenderEventDispatcher final
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
		Listener(T_RenderEventFlags const eventFlags, T_RenderEventCallback& func);

		T_RenderEventFlags flags;
		T_RenderEventCallback callback;
	};

public:

	// construct destruct
	//---------------------
	RenderEventDispatcher() = default;
	~RenderEventDispatcher() = default;

	// functionality
	//---------------
	T_RenderEventCallbackId Register(T_RenderEventFlags const flags, T_RenderEventCallback& callback);
	void Unregister(T_RenderEventCallbackId& callbackId);

	void Notify(RenderEventData const* const eventData); 

private:

	// Data
	///////

	core::slot_map<Listener> m_Listeners;
};


} // namespace render
