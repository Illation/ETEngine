#pragma once
#include "RawInputListener.h"

#include <map>

#include <EtCore/Util/Singleton.h>


namespace et {
namespace core {


//---------------------------------
// InputManager
//
// Input Manager class 
//  - doesn't do much on it's own, needs to be provided with input by a library, but abstracts it for all other engine systems
//
class InputManager final : public Singleton<InputManager>, public I_RawInputListener
{
private:
	friend class Singleton<InputManager>;
	friend class TickManager;

	// ctor dtor
	//-----------
	InputManager() : m_KeyStates() {}
	virtual ~InputManager() = default;

	// framewise
	//-------------
	void Update();

public:

	// accessors
	//-----------
	E_KeyState GetKeyState(E_KbdKey const key);

	E_KeyState GetMouseButton(E_MouseButton const button);
	vec2 const& GetMousePos() const { return m_MousePos; }
	vec2 const& GetMouseMove() const { return m_MouseMove; }
	vec2 const& GetMouseWheelDelta() const { return m_MouseWheelDelta; }

	bool IsRunning() const { return m_IsRunning; }

	// modify state
	//-----------
	void Quit() { m_IsRunning = false; }

	// Raw Input Listener interface
	//------------------------------
	bool ProcessKeyPressed(E_KbdKey const key, T_KeyModifierFlags const) override;
	bool ProcessKeyReleased(E_KbdKey const key, T_KeyModifierFlags const) override;
	bool ProcessMousePressed(E_MouseButton const button, T_KeyModifierFlags const) override;
	bool ProcessMouseReleased(E_MouseButton const button, T_KeyModifierFlags const) override;
	bool ProcessMouseMove(ivec2 const& mousePos, T_KeyModifierFlags const) override;
	bool ProcessMouseWheelDelta(ivec2 const& mouseWheel, T_KeyModifierFlags const) override;
	bool ProcessMouseEnterLeave(bool const, T_KeyModifierFlags const) override { return false; }
	bool ProcessTextInput(E_Character const) override { return false; }

	// Utility
	//-------------
private:
	void CycleKeyState(E_KeyState& state) const;

	// DATA
	///////
	bool m_IsRunning = true;

	// populated dynamically right now because I'm lazy
	std::map<E_KbdKey, E_KeyState> m_KeyStates;

	// Mouse Input
	vec2 m_MousePos;
	vec2 m_MouseMove;
	vec2 m_MouseWheelDelta;

	std::array<E_KeyState, E_MouseButton::COUNT> m_MouseButtons;
};


} // namespace core
} // namespace et
