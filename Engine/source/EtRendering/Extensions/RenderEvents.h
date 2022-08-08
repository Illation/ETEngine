#pragma once
#include <EtCore/Util/GenericEventDispatcher.h>


namespace et {
namespace render {


class rhi::I_ViewportRenderer;
	

//---------------------------
// E_RenderEvent
//
// List of events during the draw process
//
typedef uint8 T_RenderEventFlags;
enum E_RenderEvent : T_RenderEventFlags
{
	RE_Invalid = 0,

	RE_RenderDeferred	= 1 << 0,
	RE_RenderLights	= 1 << 1,
	RE_RenderForward	= 1 << 2,
	RE_RenderWorldGUI	= 1 << 3,
	RE_RenderOverlay	= 1 << 4,

	RE_All = 0xFF
};


//---------------------------
// RenderEventData
//
// Base scene event data contains only event type and source renderer, but can be derived from to provide additional data
//
struct RenderEventData
{
public:
	RenderEventData(rhi::I_ViewportRenderer const* const r, rhi::T_FbLoc const fb) : renderer(r), targetFb(fb) {}
	virtual ~RenderEventData() = default;

	rhi::I_ViewportRenderer const* renderer = nullptr;
	rhi::T_FbLoc targetFb;
};


typedef core::GenericEventDispatcher<T_RenderEventFlags, RenderEventData> T_RenderEventDispatcher;


typedef T_RenderEventDispatcher::T_CallbackId T_RenderEventCallbackId;
typedef T_RenderEventDispatcher::T_CallbackFn T_RenderEventCallback;


} // namespace render
} // namespace et
