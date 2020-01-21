#pragma once


namespace et {
namespace fw {


//---------------------------------
// E_TickOrder
//
// Order in which order various modules should be ticked
//
enum class E_TickOrder : uint32
{
	TICK_SceneManager,
	TICK_Editor,
	TICK_Framework,
	TICK_GlfwEventManager,

	COUNT
};


} // namespace fw
} // namespace et
