#pragma once
#include <EtFramework/Config/TickOrder.h>


namespace et {
namespace edit {


//---------------------------------
// E_TickOrder
//
// Order in which order various modules should be ticked
//
enum class E_EditorTickOrder : uint32
{
	TICK_SceneEditor = static_cast<E_EditorTickOrder>(fw::E_TickOrder::COUNT),
	TICK_EditorWindow,

	COUNT
};


} // namespace edit
} // namespace et
