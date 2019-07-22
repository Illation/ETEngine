#include "stdafx.h"
#include "InputManager.h"


//===================
// Input Manager
//===================


//---------------------------------
// InputManager::Update
//
// Updates key states
//
void InputManager::Update()
{
	// keyboard
	for (auto & key : m_KeyStates)
	{
		CycleKeyState(key.second);
	}

	// mouse
	for (auto & button : m_MouseButtons)
	{
		CycleKeyState(button);
	}
	m_MouseConsumed = false;
	m_MouseWheelDelta = vec2();
	m_MouseMove = vec2();
}

//---------------------------------
// InputManager::GetKeyState
//
// Returns the state of a given key
//
E_KeyState InputManager::GetKeyState(uint32 const key)
{
	auto keyIt = m_KeyStates.find(key);
	if (keyIt != m_KeyStates.cend())
	{
		return keyIt->second;
	}
	return E_KeyState::Up;
}

//---------------------------------
// InputManager::GetMouseButton
//
// Returns the state of a given mouse button
//
E_KeyState InputManager::GetMouseButton(E_MouseButton const button)
{
	if (m_MouseConsumed)
	{
		return E_KeyState::Up;
	}

	return m_MouseButtons[button];
}

//---------------------------------
// InputManager::OnKeyPressed
//
// Sets the key to pressed
//
void InputManager::OnKeyPressed(uint32 const key)
{
	m_KeyStates[key] = E_KeyState::Pressed;
}

//---------------------------------
// InputManager::OnKeyReleased
//
// sets the key to Released
//
void InputManager::OnKeyReleased(uint32 const key)
{
	m_KeyStates[key] = E_KeyState::Released;
}

//---------------------------------
// InputManager::OnMousePressed
//
// Sets the button to pressed
//
void InputManager::OnMousePressed(E_MouseButton const button)
{
	m_MouseButtons[button] = E_KeyState::Pressed;
}

//---------------------------------
// InputManager::OnMouseReleased
//
// sets the button to Released
//
void InputManager::OnMouseReleased(E_MouseButton const button)
{
	m_MouseButtons[button] = E_KeyState::Released;
}

//---------------------------------
// InputManager::OnMouseMoved
//
// updates the cursor position and velocity
//
void InputManager::OnMouseMoved(ivec2 const& mousePos)
{
	vec2 newMouse = etm::vecCast<float>(mousePos);
	m_MouseMove = newMouse - m_MousePos;
	m_MousePos = newMouse;
}

//---------------------------------
// InputManager::SetMouseWheelDelta
//
// Sets how much the user scrolled in the last frame
//
void InputManager::SetMouseWheelDelta(ivec2 const& mouseWheel)
{
	m_MouseWheelDelta = etm::vecCast<float>(mouseWheel);
}

//---------------------------------
// InputManager::CycleKeyState
//
// Updates a key state
//
void InputManager::CycleKeyState(E_KeyState& state) const
{
	if (state == E_KeyState::Pressed)
	{
		state = E_KeyState::Down;
	}
	else if (state == E_KeyState::Released)
	{
		state = E_KeyState::Up;
	}
}
