#pragma once
#include <EtCore/Containers/slot_map.h>

#include <functional>


namespace core {


//---------------------------
// GenericEventDispatcher
//
// Abstract class that can register listeners for events events using bitflags and send event data to the appropriate listeners
// Event data is sent in pointer form in order to support polymorphism
//
template <typename TFlagType, class TEventData>
class GenericEventDispatcher final
{
	// definitions
	//----------------
public:
	using T_CallbackFn = std::function<void(TFlagType const, TEventData const* const)>;
	using T_CallbackId = T_SlotId;
	static constexpr T_CallbackId INVALID_ID = INVALID_SLOT_ID;

private:
	//---------------------------
	// E_SceneEvent::Listener
	//
	// All the information needed to manage a callback
	//
	struct Listener
	{
		Listener(TFlagType const eventFlags, T_CallbackFn& func);

		TFlagType flags;
		T_CallbackFn callback;
	};

public:

	// construct destruct
	//---------------------
	GenericEventDispatcher() = default;
	~GenericEventDispatcher() = default;

	// functionality
	//---------------
	T_CallbackId Register(TFlagType const flags, T_CallbackFn& callback);
	void Unregister(T_CallbackId& callbackId);

	void Notify(TFlagType const eventType, TEventData const* const eventData);

private:

	// Data
	///////

	slot_map<Listener> m_Listeners;
};


} // namespace core


#include "GenericEventDispatcher.inl"
