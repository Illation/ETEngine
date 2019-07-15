#pragma once


//---------------------------------
// E_TickOrder
//
// Order in which order various modules should be ticked
//
enum class E_TickOrder : uint32
{
	TICK_SdlEventManager,
	TICK_Framework,
	TICK_Editor,
	TICK_SceneManager,
};

