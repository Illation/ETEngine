#pragma once
#include "KeyCodes.h"


namespace et {
namespace core {


//---------------------------------
// I_RawInputListener
//
// Interface for a class that responds to input events
//  - functions should return true if the event is consumed
//
class I_RawInputListener
{
public:
	virtual ~I_RawInputListener() = default;

	// interface
	//-----------

	// Input Listeners with higher priority get to process and consume input first
	virtual int8 GetPriority() const { return 0; }

	// keyboard
	virtual bool ProcessKeyPressed(E_KbdKey const key, T_KeyModifierFlags const modifiers) = 0;
	virtual bool ProcessKeyReleased(E_KbdKey const key, T_KeyModifierFlags const modifiers) = 0;

	// mouse
	virtual bool ProcessMousePressed(E_MouseButton const button, T_KeyModifierFlags const modifiers) = 0;
	virtual bool ProcessMouseReleased(E_MouseButton const button, T_KeyModifierFlags const modifiers) = 0;

	virtual bool ProcessMouseMove(ivec2 const& mousePos, T_KeyModifierFlags const modifiers) = 0;
	virtual bool ProcessMouseWheelDelta(ivec2 const& mouseWheel, T_KeyModifierFlags const modifiers) = 0;
};


} // namespace core
} // namespace et
