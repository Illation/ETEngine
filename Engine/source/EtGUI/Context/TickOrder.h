#pragma once


namespace et {
namespace gui {


//---------------------------------
// E_TickOrder
//
// Order in which order various modules should be ticked
//
enum class E_TickOrder : uint32
{
	TICK_GuiExtension,

	COUNT
};


} // namespace gui
} // namespace et

