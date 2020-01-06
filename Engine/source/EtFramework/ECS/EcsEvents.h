#pragma once
#include "EntityFwd.h"

#include <EtCore/Helper/GenericEventDispatcher.h>


namespace fw {


class EcsController;


namespace detail {


// generic
/////////////

//---------------------------
// E_EcsEvent
//
// List of events systems can listen for
//
typedef uint8 T_EcsEvent;
enum E_EcsEvent : T_EcsEvent
{
	Invalid = 0,

	Added		= 1 << 0,
	Removed		= 1 << 1,

	All = 0xFF
};


// component events
////////////////////

//---------------------------
// ComponentEventData
//
struct ComponentEventData
{
public:
	ComponentEventData(EcsController* const ecsController, void* const comp, T_EntityId const e) 
		: controller(ecsController), component(comp), entity(e) {}
	virtual ~ComponentEventData() = default;

	EcsController* controller = nullptr;
	void* component = nullptr;
	T_EntityId entity = INVALID_ENTITY_ID;
};

typedef core::GenericEventDispatcher<T_EcsEvent, ComponentEventData> T_ComponentEventDispatcher;

typedef T_ComponentEventDispatcher::T_CallbackFn T_ComponentEventCallbackInternal;


// entity events
/////////////////

//---------------------------
// EntityEventData
//
struct EntityEventData
{
public:
	EntityEventData(EcsController* const ecsController, T_EntityId const e)
		: controller(ecsController), entity(e) {}
	virtual ~EntityEventData() = default;

	EcsController* controller = nullptr;
	T_EntityId entity = INVALID_ENTITY_ID;
};

typedef core::GenericEventDispatcher<T_EcsEvent, EntityEventData> T_EntityEventDispatcher;

typedef T_EntityEventDispatcher::T_CallbackFn T_EntityEventCallbackInternal;


} // namespace detail


// component events
////////////////////

typedef detail::T_ComponentEventDispatcher::T_CallbackId T_CompEventId;

template <typename TComponentType>
using T_CompEventFn = std::function<void(EcsController&, TComponentType&, T_EntityId const)>;


// entity events
/////////////////

typedef detail::T_EntityEventDispatcher::T_CallbackId T_EntityEventId;
typedef std::function<void(EcsController&, T_EntityId const)> T_EntityEventFn;


} // namespace fw
