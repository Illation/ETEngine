#pragma once
#include <EtCore/Helper/GenericEventDispatcher.h>


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
	RenderLights	= 1 << 1,
	RenderForward	= 1 << 2,
	RenderOutlines	= 1 << 3,

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
	RenderEventData(I_ViewportRenderer const* const r, T_FbLoc const fb) : renderer(r), targetFb(fb) {}
	virtual ~RenderEventData() = default;

	I_ViewportRenderer const* renderer = nullptr;
	T_FbLoc targetFb;
};


typedef core::GenericEventDispatcher<T_RenderEventFlags, RenderEventData> T_RenderEventDispatcher;


typedef T_RenderEventDispatcher::T_CallbackId T_RenderEventCallbackId;
typedef T_RenderEventDispatcher::T_CallbackFn T_RenderEventCallback;


} // namespace render
