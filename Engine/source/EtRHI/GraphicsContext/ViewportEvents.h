#pragma once
#include <EtCore/Util/GenericEventDispatcher.h>


namespace et {
namespace rhi {


class Viewport;
	

//---------------------------
// E_RenderEvent
//
typedef uint8 T_ViewportEventFlags;
enum E_ViewportEvent : T_ViewportEventFlags
{
	VP_Invalid = 0,

	VP_Resized		= 1 << 0,
	VP_PreRender	= 1 << 1,
	VP_PostFlush	= 1 << 2,
	VP_NewRenderer	= 1 << 3,

	VP_All = 0xFF
};


//---------------------------
// ViewportEventData
//
struct ViewportEventData final
{
public:
	ViewportEventData(Viewport const* const v) : viewport(v) {}
	ViewportEventData(Viewport const* const v, ivec2 const s) : viewport(v), size(s) {}
	ViewportEventData(Viewport const* const v, T_FbLoc const fb) : viewport(v), targetFb(fb) {}
	virtual ~ViewportEventData() = default;

	Viewport const* const viewport = nullptr;
	ivec2 const size;
	T_FbLoc const targetFb = 0u;
};


typedef core::GenericEventDispatcher<T_ViewportEventFlags, ViewportEventData> T_ViewportEventDispatcher;


typedef T_ViewportEventDispatcher::T_CallbackId T_ViewportEventCallbackId;
typedef T_ViewportEventDispatcher::T_CallbackFn T_ViewportEventCallback;


} // namespace rhi
} // namespace et
