#pragma once
#include <map>

#include "Singleton.h"


//---------------------------------
// E_KeyState
//
// Reflects the state of a keyboard key in a simplistic way - pressed and released are only valid for one frame
//
enum E_KeyState
{
	Released,
	Up,			// Check <= up for up or released
	Down,		// Check >= down for down or pressed
	Pressed
};

//---------------------------------
// E_MouseButton
//
// Types of mouse buttons
//
enum E_MouseButton : uint8
{
	Left,
	Center,		
	Right,	

	COUNT
};

//---------------------------------
// InputManager
//
// Input Manager class
//
class InputManager : public Singleton<InputManager>
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
	E_KeyState GetKeyState(uint32 const key);

	E_KeyState GetMouseButton(E_MouseButton const button);
	vec2 const& GetMousePos() const { return m_MousePos; }
	vec2 const& GetMouseMove() const { return m_MouseMove; }
	vec2 const& GetMouseWheelDelta() const { return m_MouseWheelDelta; }

	bool IsRunning() const { return m_IsRunning; }

	// modify state
	//-----------
	void OnKeyPressed(uint32 const key);
	void OnKeyReleased(uint32 const key);
	
	void OnMousePressed(E_MouseButton const button);
	void OnMouseReleased(E_MouseButton const button);

	void OnMouseMoved(ivec2 const& mousePos);
	void SetMouseWheelDelta(ivec2 const& mouseWheel);

	void ConsumeMouse() { m_MouseConsumed = true; }

	void Quit() { m_IsRunning = false; }

	// Utility
	//-------------
private:
	void CycleKeyState(E_KeyState& state) const;

	// DATA
	///////
	bool m_IsRunning = true;

	// populated dynamically right now because I'm lazy
	std::map<uint32, E_KeyState> m_KeyStates;

	// Mouse Input
	vec2 m_MousePos;
	vec2 m_MouseMove;
	vec2 m_MouseWheelDelta;

	std::array<E_KeyState, E_MouseButton::COUNT> m_MouseButtons;

	bool m_MouseConsumed = false;
};
