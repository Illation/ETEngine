#pragma once
#include <EtGUI/Context/TickOrder.h>


namespace et {
namespace fw {


//---------------------------------
// E_TickOrder
//
// Order in which order various modules should be ticked
//
enum class E_TickOrder : uint32
{
	TICK_SceneManager = static_cast<E_TickOrder>(gui::E_TickOrder::COUNT),
	TICK_Framework,
	TICK_ImguiBackend,
	TICK_GlfwEventManager,

	COUNT
};


} // namespace fw
} // namespace et
