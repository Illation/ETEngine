#pragma once
#include <map>
#include <SDL.h>
//Use lefthanded coordinate system
#ifndef GLM_LEFT_HANDED
	#define GLM_LEFT_HANDED
#endif
#include <glm\glm.hpp>
#include "../Helper/Singleton.h"
//----------------------------
//Event Manager class definintion
//----------------------------
class InputManager : public Singleton<InputManager>
{
public:
	//Constructor Destructor
	InputManager();
	virtual ~InputManager();
	//----------------------------
	//Member functions
	//----------------------------
	//Keyboard

	// True if user starts pressing key
	bool IsKeyboardKeyPressed(SDL_Scancode key);
	// True if user starts pressing key
	// Supported chars are CAPITAL letters and numbers
	bool IsKeyboardKeyPressed(char key);
	// True if user is pressing key
	bool IsKeyboardKeyDown(SDL_Scancode key);
	// True if user is pressing key
	// Supported chars are CAPITAL letters and numbers
	bool IsKeyboardKeyDown(char key);
	// True if user stops pressing key
	bool IsKeyboardKeyReleased(SDL_Scancode key);
	// True if user stops pressing key
	// Supported chars are CAPITAL letters and numbers
	bool IsKeyboardKeyReleased(char key);

	//Mouse

	//True if user starts pressing button
	bool IsMouseButtonPressed(int32 button);
	//True if user is pressing button
	bool IsMouseButtonDown(int32 button);
	//True if user stops pressing button
	bool IsMouseButtonReleased(int32 button);
	//Returns mouse position as vector
	glm::vec2 GetMousePosition();
	glm::vec2 GetMouseMovement() { return m_MouseMove; }
	glm::vec2 GetMouseWheelDelta() { return m_MouseWheelDelta; }
private:
	friend class AbstractFramework;
	//----------------------------
	// Member functions
	//----------------------------

	//General
	void Init();//call after SDL init
	void UpdateEvents();//call before all GUI ticks
						//Getters

	bool IsExitRequested();

	// Gets the SDL_Scancode for a char
	// Returns false if char is not mapped
	bool GetScancode(SDL_Scancode &code, char key);

	//----------------------------
	// Data Members
	//----------------------------

	//Key Input
	uint8 *m_KeyMapNew = nullptr,
		*m_KeyMapOld = nullptr;
	const Uint8 *m_KeyMapSdl = nullptr;
	int32 m_KeyboardLength;
	std::map<char, SDL_Scancode> m_CharToSdlMap;

	//Mouse Input
	int32 m_MousePosX = 0;
	int32 m_MousePosY = 0;
	glm::vec2 m_MouseMove = glm::vec2();
	uint32 m_MouseMapNew,
		m_MouseMapOld;

	glm::vec2 m_MouseWheelDelta = glm::vec2();
	//Application flow
	bool m_ExitRequested = false;
};

