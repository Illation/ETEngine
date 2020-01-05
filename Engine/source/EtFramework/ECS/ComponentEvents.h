#pragma once
#include <EtCore/Helper/GenericEventDispatcher.h>


namespace framework {


class EcsController;


namespace detail {


//---------------------------
// E_ComponentEvent
//
// List of events systems can listen for
//
typedef uint8 T_ComponentEvent;
enum E_ComponentEvent : T_ComponentEvent
{
	Invalid = 0,

	Added		= 1 << 0,
	Removed		= 1 << 1,

	All = 0xFF
};

//---------------------------
// ComponentEventData
//
struct ComponentEventData
{
public:
	ComponentEventData(EcsController* const ecsController, void* const comp) : controller(ecsController), component(comp) {}
	virtual ~ComponentEventData() = default;

	EcsController* controller = nullptr;
	void* component = nullptr;
};


typedef core::GenericEventDispatcher<T_ComponentEvent, ComponentEventData> T_ComponentEventDispatcher;

typedef T_ComponentEventDispatcher::T_CallbackFn T_ComponentEventCallbackInternal;


} // namespace detail


typedef detail::T_ComponentEventDispatcher::T_CallbackId T_ComEventId;


} // namespace framework
