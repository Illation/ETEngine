#include "stdafx.h"
#include "InputManager.h"


namespace et {
namespace core {


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

	m_MouseWheelDelta = vec2();
	m_MouseMove = vec2();
}

//---------------------------------
// InputManager::GetKeyState
//
// Returns the state of a given key
//
E_KeyState InputManager::GetKeyState(E_KbdKey const key)
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
	return m_MouseButtons[button];
}

//---------------------------------
// InputManager::ProcessKeyPressed
//
bool InputManager::ProcessKeyPressed(E_KbdKey const key, T_KeyModifierFlags const)
{
	m_KeyStates[key] = E_KeyState::Pressed;
	return true;
}

//----------------------------------
// InputManager::ProcessKeyReleased
//
bool InputManager::ProcessKeyReleased(E_KbdKey const key, T_KeyModifierFlags const)
{
	m_KeyStates[key] = E_KeyState::Released;
	return true;
}

//-----------------------------------
// InputManager::ProcessMousePressed
//
bool InputManager::ProcessMousePressed(E_MouseButton const button, T_KeyModifierFlags const)
{
	m_MouseButtons[button] = E_KeyState::Pressed;
	return true;
}

//------------------------------------
// InputManager::ProcessMouseReleased
//
bool InputManager::ProcessMouseReleased(E_MouseButton const button, T_KeyModifierFlags const)
{
	m_MouseButtons[button] = E_KeyState::Released;
	return true;
}

//--------------------------------
// InputManager::ProcessMouseMove
//
bool InputManager::ProcessMouseMove(ivec2 const& mousePos, T_KeyModifierFlags const)
{
	vec2 newMouse = math::vecCast<float>(mousePos);
	m_MouseMove = newMouse - m_MousePos;
	m_MousePos = newMouse;
	return true;
}

//--------------------------------------
// InputManager::ProcessMouseWheelDelta
//
bool InputManager::ProcessMouseWheelDelta(ivec2 const& mouseWheel, T_KeyModifierFlags const)
{
	m_MouseWheelDelta = math::vecCast<float>(mouseWheel);
	return true;
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


} // namespace core
} // namespace et
